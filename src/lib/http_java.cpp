// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>
#include "common_java.hpp"
#include "http_impl.hpp"

#include <jni.h>

#include <stddef.h>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace brigid {
  namespace {
    jclass clazz;
    class vtable {
    public:
      vtable()
        : set_credential(clazz, "setCredential", "([B[B)V"),
          reset_credential(clazz, "resetCredential", "()V"),
          constructor(clazz, "([B[B)V"),
          set_header(clazz, "setHeader", "([B[B)V"),
          send(clazz, "send", "()V"),
          send_body(clazz, "sendBody", "(J)V"),
          write(clazz, "write", "([BII)V"),
          recv(clazz, "recv", "()I"),
          get_response_code(clazz, "getResponseCode", "()I"),
          get_header_key(clazz, "getHeaderKey", "(I)[B"),
          get_header_value(clazz, "getHeaderValue", "(I)[B"),
          read(clazz, "read", "([B)I"),
          close(clazz, "close", "()V") {}

      static_method<void> set_credential;
      static_method<void> reset_credential;
      constructor_method constructor;
      method<void> set_header;
      method<void> send;
      method<void> send_body;
      method<void> write;
      method<jint> recv;
      method<jint> get_response_code;
      method<jbyteArray> get_header_key;
      method<jbyteArray> get_header_value;
      method<jint> read;
      method<void> close;
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
        : progress_cb(progress_cb),
          header_cb(header_cb),
          write_cb(write_cb),
          credential(credential),
          jbuffer(make_global_ref<jbyteArray>()) {
        if (credential) {
          vt.set_credential(clazz, make_byte_array(username), make_byte_array(password));
        }
      }

      ~http_session_impl() {
        try {
          if (credential) {
            vt.reset_credential(clazz);
          }
        } catch (...) {}
      }

      void ensure_buffer_size(size_t size) {
        if (nbuffer.size() < size) {
          nbuffer.resize(size);
          jbuffer = make_global_ref(make_byte_array(size));
        }
      }

      virtual bool request(const std::string&, const std::string&, const std::map<std::string, std::string>&, http_request_body, const char*, size_t);

      vtable vt;
      std::function<bool (size_t, size_t)> progress_cb;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb;
      std::function<bool (const char*, size_t)> write_cb;
      bool credential;
      std::vector<char> nbuffer;
      global_ref_t<jbyteArray> jbuffer;
    };

    class http_task : private noncopyable {
    public:
      http_task(http_session_impl& session)
        : session_(session),
          instance_(make_global_ref<jobject>()) {}

      ~http_task() {
        try {
          if (instance_) {
            session_.vt.close(instance_);
          }
        } catch (...) {}
      }

      bool request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data,
          size_t size) {
        instance_ = make_global_ref(session_.vt.constructor(clazz, make_byte_array(method), make_byte_array(url)));

        for (const auto& field : header) {
          session_.vt.set_header(instance_, make_byte_array(field.first), make_byte_array(field.second));
        }

        if (std::unique_ptr<http_reader> reader = make_http_reader(body, data, size)) {
          session_.vt.send_body(instance_, reader->total());

          session_.ensure_buffer_size(http_buffer_size);
          while (true) {
            size_t result = reader->read(session_.nbuffer.data(), session_.nbuffer.size());
            if (result == 0) {
              break;
            }
            set_byte_array_region(session_.jbuffer, 0, result, session_.nbuffer.data());

            session_.vt.write(instance_, session_.jbuffer, 0, result);

            if (!session_.progress_cb(reader->now(), reader->total())) {
              return false;
            }
          }
        } else {
          session_.vt.send(instance_);
        }

        session_.vt.recv(instance_);

        {
          jint code = session_.vt.get_response_code(instance_);

          std::map<std::string, std::string> header;
          for (jint i = 0; ; ++i) {
            local_ref_t<jbyteArray> value = session_.vt.get_header_value(instance_, i);
            if (!value) {
              break;
            }
            if (local_ref_t<jbyteArray> key = session_.vt.get_header_key(instance_, i)) {
              header[get_byte_array_region(key)] = get_byte_array_region(value);
            }
          }

          if (!session_.header_cb(code, header)) {
            return false;
          }
        }

        session_.ensure_buffer_size(http_buffer_size);
        while (true) {
          jint result = session_.vt.read(instance_, session_.jbuffer);
          if (result < 0) {
            break;
          }
          if (result > 0) {
            get_byte_array_region(session_.jbuffer, 0, result, session_.nbuffer.data());

            if (!session_.write_cb(session_.nbuffer.data(), result)) {
              return false;
            }
          }
        }

        return true;
      }

    private:
      http_session_impl& session_;
      global_ref_t<jobject> instance_;
    };

    bool http_session_impl::request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& header,
        http_request_body body,
        const char* data,
        size_t size) {
      http_task task(*this);
      return task.request(method, url, header, body, data, size);
    }

    std::mutex mutex;
  }

  http_initializer::http_initializer() {}
  http_initializer::~http_initializer() {}

  void open_http() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!clazz) {
      clazz = make_global_ref(find_class("jp/brigid/HttpTask")).release();
    }
  }

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
