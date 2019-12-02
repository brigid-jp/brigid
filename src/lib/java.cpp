// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "error.hpp"
#include "java.hpp"

#include <jni.h>

#include <vector>

extern "C" void* SDL_AndroidGetJNIEnv();

namespace brigid {
  namespace java {
    JNIEnv* get_env() {
      return static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    }

    void delete_local_ref(jobject object) {
      get_env()->DeleteLocalRef(object);
    }

    void delete_global_ref(jobject object) {
      get_env()->DeleteGlobalRef(object);
    }

    void check_impl(bool null_pointer) {
      JNIEnv* env = get_env();
      if (!env->ExceptionCheck()) {
        if (null_pointer) {
          throw BRIGID_ERROR("null pointer");
        }
        return;
      }
      local_ref_t<jthrowable> instance = make_local_ref(env->ExceptionOccurred());
      env->ExceptionClear();

      local_ref_t<jclass> klass = make_local_ref(env->FindClass("java/lang/Throwable"));
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        klass.reset();
      }
      if (!klass) {
        throw BRIGID_ERROR("cannot FindClass");
      }

      jmethodID method = env->GetMethodID(klass.get(), "toString", "()Ljava/lang/String;");
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        method = nullptr;
      }
      if (!method) {
        throw BRIGID_ERROR("cannot GetMethodID");
      }

      local_ref_t<jstring> result = make_local_ref(reinterpret_cast<jstring>(env->CallObjectMethod(instance.get(), method)));
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        result.reset();
      }
      if (!result) {
        throw BRIGID_ERROR("cannot CallObjectMethod");
      }

      std::vector<char> buffer(env->GetStringUTFLength(result.get()) + 1);
      env->GetStringUTFRegion(result.get(), 0, env->GetStringLength(result.get()), buffer.data());
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        throw BRIGID_ERROR("cannot GetStringUTFRegion");
      }

      throw BRIGID_ERROR(buffer.data());
    }

    void check() {
      check_impl(false);
    }

    jboolean to_boolean(bool source) {
      if (source) {
        return JNI_TRUE;
      } else {
        return JNI_FALSE;
      }
    }
  }
}