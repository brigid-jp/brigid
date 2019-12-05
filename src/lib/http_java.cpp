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

namespace brigid {
  namespace {
    using namespace java;

    class vtable {
    public:
      vtable()
        : clazz(make_global_ref(find_class("jp/brigid/HttpTask"))),
          set_credential(clazz, "setCredential", "([B[B)V"),
          reset_credential(clazz, "resetCredential", "()V"),
          construct(clazz, "([B[B)V"),
          set_header(clazz, "setHeader", "([B[B)V"),
          connect(clazz, "connect", "(J)V"),
          write(clazz, "write", "([BII)V"),
          get_response_code(clazz, "getResponseCode", "()I"),
          get_header_field_key(clazz, "getHeaderFieldKey", "(I)[B"),
          get_header_field(clazz, "getHeaderField", "(I)[B"),
          read(clazz, "read", "([B)I"),
          disconnect(clazz, "disconnect", "()V") {}

      global_ref_t<jclass> clazz;
      static_method<void> set_credential;
      static_method<void> reset_credential;
      constructor_method construct;
      method<void> set_header;
      method<void> connect;
      method<void> write;
      method<jint> get_response_code;
      method<jbyteArray> get_header_field_key;
      method<jbyteArray> get_header_field;
      method<jint> read;
      method<void> disconnect;
    };

    class http_session_impl : public http_session, private noncopyable {
    public:
      http_session_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : buffer(make_global_ref(make_byte_array(http_buffer_size))),
          progress_cb(progress_cb),
          header_cb(header_cb),
          write_cb(write_cb),
          credential(credential) {
        if (credential) {
          vt.set_credential(vt.clazz, make_byte_array(username), make_byte_array(password));
        }
      }

      ~http_session_impl() {
        try {
          if (credential) {
            vt.reset_credential(vt.clazz);
          }
        } catch (...) {}
      }

      virtual void request(const std::string&, const std::string&, const std::map<std::string, std::string>&, http_request_body, const char*, size_t);

      vtable vt;
      global_ref_t<jbyteArray> buffer;
      std::function<bool (size_t, size_t)> progress_cb;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb;
      std::function<bool (const char*, size_t)> write_cb;
      bool credential;
    };

    class http_task : private noncopyable {
    public:
      http_task(
          http_session_impl& session,
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header)
        : session_(session),
          instance_(make_global_ref(session_.vt.construct(
              session_.vt.clazz,
              make_byte_array(method),
              make_byte_array(url)))) {
        for (const auto& field : header) {
          session_.vt.set_header(instance_, make_byte_array(field.first), make_byte_array(field.second));
        }
      }

      void request(http_request_body body, const char* data, size_t size) {
        if (std::unique_ptr<http_reader> reader = make_http_reader(body, data, size)) {
          session_.vt.connect(instance_, reader->total());
          while (true) {
            std::vector<char> buffer(http_buffer_size);
            size_t result = reader->read(buffer.data(), buffer.size());
            if (result == 0) {
              break;
            }
            set_byte_array_region(session_.buffer, 0, result, buffer.data());

            session_.vt.write(instance_, session_.buffer, 0, result);

            if (session_.progress_cb) {
              if (!session_.progress_cb(reader->now(), reader->total())) {
                throw BRIGID_ERROR("canceled");
              }
            }
          }
        } else {
          session_.vt.connect(instance_, -1);
        }

        jint code = session_.vt.get_response_code(instance_);
        if (session_.header_cb) {
          std::map<std::string, std::string> header;
          for (jint i = 0; ; ++i) {
            local_ref_t<jbyteArray> value = session_.vt.get_header_field(instance_, i);
            if (!value) {
              break;
            }
            if (local_ref_t<jbyteArray> key = session_.vt.get_header_field_key(instance_, i)) {
              header[get_byte_array_region(key)] = get_byte_array_region(value);
            }
          }

          if (!session_.header_cb(code, header)) {
            throw BRIGID_ERROR("canceled");
          }
        }

        if (session_.write_cb) {
          while (true) {
            jint result = session_.vt.read(instance_, session_.buffer);
            if (result < 0) {
              break;
            }
            if (result > 0) {
              std::vector<char> buffer(result);
              get_byte_array_region(session_.buffer, 0, result, buffer.data());

              if (!session_.write_cb(buffer.data(), buffer.size())) {
                throw BRIGID_ERROR("canceled");
              }
            }
          }
        }

        session_.vt.disconnect(instance_);
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
      bool credential,
      const std::string& username,
      const std::string& password) {
    return std::unique_ptr<http_session>(new http_session_impl(progress_cb, header_cb, write_cb, credential, username, password));
  }
}
