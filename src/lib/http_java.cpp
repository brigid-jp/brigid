// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>
#include "util_java.hpp"

#include <jni.h>

#include <stddef.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

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
        : klass_(make_global_ref<jclass>()),
          instance_(make_global_ref<jobject>()),
          progress_cb_(progress_cb),
          header_cb_(header_cb),
          write_cb_(write_cb) {
        JNIEnv* env = get_env();

        local_ref_t<jclass> klass = make_local_ref(check(env->FindClass("jp/brigid/HttpSession")));
        klass_ = make_global_ref(check(reinterpret_cast<jclass>(env->NewGlobalRef(klass.get()))));

        {
          jmethodID method = check(env->GetMethodID(klass.get(), "<init>", "(I[B[B)V"));

          local_ref_t<jbyteArray> username_ = make_local_ref(check(env->NewByteArray(username.size())));
          env->SetByteArrayRegion(username_.get(), 0, username.size(), reinterpret_cast<const jbyte*>(username.data()));
          check();

          local_ref_t<jbyteArray> password_ = make_local_ref(check(env->NewByteArray(password.size())));
          env->SetByteArrayRegion(password_.get(), 0, password.size(), reinterpret_cast<const jbyte*>(password.data()));
          check();

          local_ref_t<jobject> instance = make_local_ref(check(env->NewObject(klass.get(), method, static_cast<jint>(auth_scheme), username_.get(), password_.get())));
        }
      }

      virtual void request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data,
          size_t size) {
      }

    private:
      global_ref_t<jclass> klass_;
      global_ref_t<jobject> instance_;
      std::function<bool (size_t, size_t)> progress_cb_;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
    };
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
