// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "java.hpp"

#include <string>
#include <vector>

extern "C" void* SDL_AndroidGetJNIEnv();

namespace brigid {
  JNIEnv* java_env() {
    return static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
  }

  void java_delete_local_ref(jobject that) {
    java_env()->DeleteLocalRef(that);
  }

  void java_delete_global_ref(jobject that) {
    java_env()->DeleteGlobalRef(that);
  }

  void java_check() {
    static const char* what = "crypto_java exception";

    JNIEnv* env = java_env();
    if (!env->ExceptionCheck()) {
      return;
    }
    java_local_ref_t<jthrowable> exception = java_make_local_ref(env->ExceptionOccurred());
    env->ExceptionClear();

    java_local_ref_t<jclass> klass = java_make_local_ref(env->FindClass("java/lang/Throwable"));
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      klass.reset();
    }
    if (!klass) {
      throw std::runtime_error(what);
    }

    jmethodID method = env->GetMethodID(klass.get(), "toString", "()Ljava/lang/String;");
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      method = nullptr;
    }
    if (!method) {
      throw std::runtime_error(what);
    }

    java_local_ref_t<jstring> result = java_make_local_ref(reinterpret_cast<jstring>(env->CallObjectMethod(exception.get(), method)));
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      result.reset();
    }
    if (!result) {
      throw std::runtime_error(what);
    }

    std::vector<char> buffer(env->GetStringUTFLength(result.get()));
    env->GetStringUTFRegion(result.get(), 0, env->GetStringLength(result.get()), buffer.data());
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      throw std::runtime_error(what);
    }

    throw std::runtime_error("crypto_java exception " + std::string(buffer.begin(), buffer.end()));
  }

  jboolean java_boolean(bool value) {
    if (value) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }
}
