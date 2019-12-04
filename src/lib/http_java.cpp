// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>
#include "http_impl.hpp"
#include "util_java.hpp"

#include <jni.h>

#include <stddef.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <iostream>

namespace brigid {
  namespace {
    using namespace java;

    class http_session_impl : public http_session, private noncopyable {
    public:
      http_session_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          http_authentication_scheme auth_scheme,
          const std::string& username,
          const std::string& password)
        : klass(make_global_ref<jclass>()),
          method_init(),
          method_set_header(),
          method_connect(),
          method_write(),
          method_get_response_code(),
          method_get_header_field_key(),
          method_get_header_field(),
          method_read(),
          method_disconnect(),
          buffer(make_global_ref<jbyteArray>()),
          progress_cb(progress_cb),
          header_cb(header_cb),
          write_cb(write_cb),
          auth_scheme(auth_scheme),
          username(username),
          password(password) {
        JNIEnv* env = get_env();

        local_ref_t<jclass> klass_ = make_local_ref(check(env->FindClass("jp/brigid/HttpTask")));
        klass = make_global_ref(check(reinterpret_cast<jclass>(env->NewGlobalRef(klass_.get()))));
        method_init = check(env->GetMethodID(klass.get(), "<init>", "(I[B[B[B[B)V"));
        method_set_header = check(env->GetMethodID(klass.get(), "setHeader", "([B[B)V"));
        method_connect = check(env->GetMethodID(klass.get(), "connect", "(J)V"));
        method_write = check(env->GetMethodID(klass.get(), "write", "([BI)V"));
        method_get_response_code = check(env->GetMethodID(klass.get(), "getResponseCode", "()I"));
        method_get_header_field_key = check(env->GetMethodID(klass.get(), "getHeaderFieldKey", "(I)[B"));
        method_get_header_field = check(env->GetMethodID(klass.get(), "getHeaderField", "(I)[B"));
        method_read = check(env->GetMethodID(klass.get(), "read", "([B)I"));
        method_disconnect = check(env->GetMethodID(klass.get(), "disconnect", "()V"));
        local_ref_t<jbyteArray> buffer_ = make_local_ref(check(env->NewByteArray(http_buffer_size)));
        buffer = make_global_ref(check(reinterpret_cast<jbyteArray>(env->NewGlobalRef(buffer_.get()))));
      }

      virtual void request(const std::string&, const std::string&, const std::map<std::string, std::string>&, http_request_body, const char*, size_t);

      global_ref_t<jclass> klass;
      jmethodID method_init;
      jmethodID method_set_header;
      jmethodID method_connect;
      jmethodID method_write;
      jmethodID method_get_response_code;
      jmethodID method_get_header_field_key;
      jmethodID method_get_header_field;
      jmethodID method_read;
      jmethodID method_disconnect;
      global_ref_t<jbyteArray> buffer;
      std::function<bool (size_t, size_t)> progress_cb;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb;
      std::function<bool (const char*, size_t)> write_cb;
      http_authentication_scheme auth_scheme;
      std::string username;
      std::string password;
    };

    class http_task : private noncopyable {
    public:
      http_task(
          http_session_impl& session,
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header)
        : session_(session),
          instance_(make_global_ref<jobject>()) {
        JNIEnv* env = get_env();

        local_ref_t<jbyteArray> username = make_byte_array(session.username);
        local_ref_t<jbyteArray> password = make_byte_array(session.password);
        local_ref_t<jbyteArray> method_ = make_byte_array(method);
        local_ref_t<jbyteArray> url_ = make_byte_array(url);

        local_ref_t<jobject> instance = make_local_ref(check(env->NewObject(
            session_.klass.get(),
            session_.method_init,
            static_cast<int>(session_.auth_scheme),
            username.get(),
            password.get(),
            method_.get(),
            url_.get())));
        instance_ = make_global_ref(check(env->NewGlobalRef(instance.get())));

        for (const auto& field : header) {
          local_ref_t<jbyteArray> key = make_byte_array(field.first);
          local_ref_t<jbyteArray> value = make_byte_array(field.second);
          env->CallVoidMethod(instance_.get(), session_.method_set_header, key.get(), value.get());
          check();
        }
      }

      void request(http_request_body body, const char* data, size_t size) {
        JNIEnv* env = get_env();

        std::unique_ptr<http_reader> reader(make_http_reader(body, data, size));

        env->CallVoidMethod(instance_.get(), session_.method_connect, reader ? reader->total() : -1);
        check();

        if (reader) {
          // size_t buffer_size = std::min(http_buffer_size, reader->total());
          // if (session_.buffer.size() < buffer_size) {
          //   session_.buffer.resize(buffer_size);
          // }

          while (true) {
            std::vector<char> buffer(http_buffer_size);
            size_t result = reader->read(buffer.data(), buffer.size());
            if (result == 0) {
              break;
            }
            env->SetByteArrayRegion(session_.buffer.get(), 0, result, reinterpret_cast<const jbyte*>(buffer.data()));
            check();
            env->CallVoidMethod(instance_.get(), session_.method_write, session_.buffer.get(), result);
            check();

            if (session_.progress_cb) {
              if (!session_.progress_cb(reader->now(), reader->total())) {
                throw BRIGID_ERROR("canceled");
              }
            }
          }
        }

        if (session_.header_cb) {
          jint code = env->CallIntMethod(instance_.get(), session_.method_get_response_code);
          check();

          std::map<std::string, std::string> header;
          for (jint i = 0; ; ++i) {
            local_ref_t<jbyteArray> field = make_local_ref<jbyteArray>(reinterpret_cast<jbyteArray>(env->CallObjectMethod(instance_.get(), session_.method_get_header_field, i)));
            check();
            if (!field) {
              break;
            }
            local_ref_t<jbyteArray> key = make_local_ref<jbyteArray>(reinterpret_cast<jbyteArray>(env->CallObjectMethod(instance_.get(), session_.method_get_header_field_key, i)));
            check();
            if (key) {
              header[to_string(key.get())] = to_string(field.get());
            }
          }

          if (!session_.header_cb(code, header)) {
            throw BRIGID_ERROR("canceled");
          }
        }

        if (session_.write_cb) {
          // if (session_.buffer.size() < http_buffer_size) {
          //   session_.buffer.resize(http_buffer_size);
          // }

          // local_ref_t<jobject> buffer = make_local_ref(check(env->NewDirectByteBuffer(const_cast<char*>(session_.buffer.data()), session_.buffer.size())));

          while (true) {
            jint result = env->CallIntMethod(instance_.get(), session_.method_read, session_.buffer.get());
            check();
            if (result < 0) {
              break;
            }
            if (result > 0) {
              std::vector<char> buffer(result);
              env->GetByteArrayRegion(session_.buffer.get(), 0, result, reinterpret_cast<jbyte*>(buffer.data()));
              check();

              if (!session_.write_cb(buffer.data(), buffer.size())) {
                throw BRIGID_ERROR("canceled");
              }
            }
          }
        }

        env->CallVoidMethod(instance_.get(), session_.method_disconnect);
        check();
      }

    private:
      http_session_impl& session_;
      global_ref_t<jobject> instance_;
    };

    void http_session_impl::request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& header,
        http_request_body body,
        const char* data,
        size_t size) {
      http_task task(*this, method, url, header);
      task.request(body, data, size);
    }
  }

  http_initializer::http_initializer() {}
  http_initializer::~http_initializer() {}

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (size_t, size_t)> progress_cb,
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      http_authentication_scheme auth_scheme,
      const std::string& username,
      const std::string& password) {
    return std::unique_ptr<http_session>(new http_session_impl(progress_cb, header_cb, write_cb, auth_scheme, username, password));
  }
}
