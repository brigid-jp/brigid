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
  namespace java {
    JNIEnv* get_env();

    void delete_local_ref(jobject);

    template <class T>
    using local_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&delete_local_ref)>;

    template <class T>
    inline local_ref_t<T> make_local_ref(T object = nullptr) {
      return local_ref_t<T>(object, &delete_local_ref);
    }

    void delete_global_ref(jobject);

    template <class T>
    using global_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&delete_global_ref)>;

    template <class T>
    inline global_ref_t<T> make_global_ref(T object = nullptr) {
      return global_ref_t<T>(object, &delete_global_ref);
    }

    void check();

    template <class T>
    T check(T result) {
      check();
      if (!result) {
        throw std::runtime_error("crypto_java exception");
      }
      return result;
    }

    jboolean to_boolean(bool);
  }
}

#endif
