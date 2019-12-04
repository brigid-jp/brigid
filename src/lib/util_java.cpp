// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "error.hpp"
#include "util_java.hpp"

#include <jni.h>

extern "C" void* SDL_AndroidGetJNIEnv();

#include <stddef.h>
#include <string>
#include <vector>

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

    local_ref_t<jbyteArray> make_byte_array(size_t size) {
      local_ref_t<jbyteArray> result = make_local_ref(check(get_env()->NewByteArray(size)));
      check();
      return result;
    }

    local_ref_t<jbyteArray> make_byte_array(const char* data, size_t size) {
      JNIEnv* env = get_env();

      local_ref_t<jbyteArray> result = make_local_ref(check(env->NewByteArray(size)));
      env->SetByteArrayRegion(result.get(), 0, size, reinterpret_cast<const jbyte*>(data));
      check();
      return result;
    }

    local_ref_t<jbyteArray> make_byte_array(const std::string& source) {
      return make_byte_array(source.data(), source.size());
    }

    std::string to_string(jbyteArray source) {
      JNIEnv* env = get_env();

      std::vector<char> buffer(env->GetArrayLength(source));
      env->GetByteArrayRegion(source, 0, buffer.size(), reinterpret_cast<jbyte*>(buffer.data()));
      check();

      return std::string(buffer.data(), buffer.size());
    }

    local_ref_t<jobject> make_direct_byte_buffer(char* data, size_t size) {
      return make_local_ref(check(get_env()->NewDirectByteBuffer(const_cast<char*>(data), size)));
    }

    local_ref_t<jclass> find_class(const char* name) {
      return make_local_ref(check(get_env()->FindClass(name)));
    }
  }
}
