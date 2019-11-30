// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_JAVA_HPP
#define BRIGID_JAVA_HPP

#include "error.hpp"
#include "type_traits.hpp"

#include <jni.h>

#include <memory>

namespace brigid {
  JNIEnv* java_env();

  void java_delete_local_ref(jobject);

  template <class T>
  using java_local_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&java_delete_local_ref)>;

  template <class T>
  inline java_local_ref_t<T> java_make_local_ref(T object = nullptr) {
    return java_local_ref_t<T>(object, &java_delete_local_ref);
  }

  void java_delete_global_ref(jobject);

  template <class T>
  using java_global_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&java_delete_global_ref)>;

  template <class T>
  inline java_global_ref_t<T> java_make_global_ref(T object = nullptr) {
    return java_global_ref_t<T>(object, &java_delete_global_ref);
  }

  void java_check();

  template <class T>
  T java_check(T that) {
    java_check();
    if (!that) {
      throw std::runtime_error("crypto_java exception");
    }
    return that;
  }

  jboolean java_boolean(bool);
}

#endif
