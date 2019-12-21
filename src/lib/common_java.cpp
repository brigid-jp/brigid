// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common_java.hpp"

#include <jni.h>

extern "C" void* SDL_AndroidGetJNIEnv();

#include <stddef.h>
#include <string>
#include <vector>

namespace brigid {
  JNIEnv* get_env() {
    return static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
  }

  void check_impl(bool null_pointer) {
    JNIEnv* env = get_env();
    if (!env->ExceptionCheck()) {
      if (null_pointer) {
        throw BRIGID_RUNTIME_ERROR("null pointer");
      }
      return;
    }
    local_ref_t<jthrowable> instance = make_local_ref(env->ExceptionOccurred());
    env->ExceptionClear();

    local_ref_t<jclass> clazz = make_local_ref(env->FindClass("java/lang/Throwable"));
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      clazz.reset();
    }
    if (!clazz) {
      throw BRIGID_LOGIC_ERROR("cannot FindClass");
    }

    jmethodID method = env->GetMethodID(clazz.get(), "toString", "()Ljava/lang/String;");
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      method = nullptr;
    }
    if (!method) {
      throw BRIGID_LOGIC_ERROR("cannot GetMethodID");
    }

    local_ref_t<jstring> result = make_local_ref(reinterpret_cast<jstring>(env->CallObjectMethod(instance.get(), method)));
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      result.reset();
    }
    if (!result) {
      throw BRIGID_LOGIC_ERROR("cannot CallObjectMethod");
    }

    std::vector<char> buffer(env->GetStringUTFLength(result.get()) + 1);
    env->GetStringUTFRegion(result.get(), 0, env->GetStringLength(result.get()), buffer.data());
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      throw BRIGID_LOGIC_ERROR("cannot GetStringUTFRegion");
    }

    throw BRIGID_RUNTIME_ERROR(buffer.data());
  }

  void check() {
    check_impl(false);
  }

  void delete_local_ref_t::operator()(jobject object) const {
    get_env()->DeleteLocalRef(object);
  }

  void delete_global_ref_t::operator()(jobject object) const {
    get_env()->DeleteGlobalRef(object);
  }

  local_ref_t<jbyteArray> make_byte_array(size_t size) {
    local_ref_t<jbyteArray> result = make_local_ref(get_env()->NewByteArray(size));
    check(result.get());
    return result;
  }

  local_ref_t<jbyteArray> make_byte_array(const char* data, size_t size) {
    local_ref_t<jbyteArray> result = make_byte_array(size);
    set_byte_array_region(result, 0, size, data);
    return result;
  }

  local_ref_t<jbyteArray> make_byte_array(const std::string& source) {
    return make_byte_array(source.data(), source.size());
  }

  local_ref_t<jobject> make_direct_byte_buffer(void* data, size_t size) {
    local_ref_t<jobject> result = make_local_ref(get_env()->NewDirectByteBuffer(data, size));
    check(result.get());
    return result;
  }

  jboolean to_boolean(bool source) {
    if (source) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }

  local_ref_t<jclass> find_class(const char* name) {
    local_ref_t<jclass> result = make_local_ref(get_env()->FindClass(name));
    check(result.get());
    return result;
  }
}
