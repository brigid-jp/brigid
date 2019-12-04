// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_JAVA_HPP
#define BRIGID_JAVA_HPP

#include <brigid/noncopyable.hpp>
#include "error.hpp"
#include "type_traits.hpp"

#include <jni.h>

#include <stddef.h>
#include <memory>
#include <string>
#include <type_traits>

namespace brigid {
  namespace java {
    template <bool T_condition, class T = void>
    using enable_if_t = typename std::enable_if<T_condition, T>::type;

    template <class T>
    using enable_if_jobject_t = enable_if_t<std::is_base_of<remove_pointer_t<jobject>, remove_pointer_t<T>>::value>;

    JNIEnv* get_env();

    void delete_local_ref(jobject);

    template <class T>
    using local_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&delete_local_ref)>;

    template <class T>
    inline local_ref_t<T> make_local_ref(T object, enable_if_jobject_t<T>* = nullptr) {
      return local_ref_t<T>(object, &delete_local_ref);
    }

    void delete_global_ref(jobject);

    template <class T>
    using global_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&delete_global_ref)>;

    template <class T>
    inline global_ref_t<T> make_global_ref(enable_if_jobject_t<T>* = nullptr) {
      return global_ref_t<T>(nullptr, &delete_global_ref);
    }

    template <class T>
    inline global_ref_t<T> make_global_ref(T object, enable_if_jobject_t<T>* = nullptr) {
      return global_ref_t<T>(object, &delete_global_ref);
    }

    void check_impl(bool);

    void check();

    template <class T>
    T* check(T* result) {
      check_impl(!result);
      return result;
    }

    template <class T>
    inline global_ref_t<typename T::pointer> make_global_ref(const T& source, enable_if_jobject_t<typename T::pointer>* = nullptr) {
      return make_global_ref(reinterpret_cast<typename T::pointer>(check(get_env()->NewGlobalRef(source.get()))));
    }

    template <class T>
    struct unref_impl {
      using type = T;

      static type unref(const T& source) {
        return source;
      }
    };

    template <class T, class T_deleter>
    struct unref_impl<std::unique_ptr<T, T_deleter>> {
      using type = T*;

      static type unref(const std::unique_ptr<T, T_deleter>& source) {
        return source.get();
      }
    };

    template <class T>
    inline typename unref_impl<T>::type unref(const T& source) {
      return unref_impl<T>::unref(source);
    }

    jboolean to_boolean(bool);

    local_ref_t<jbyteArray> make_byte_array(size_t);
    local_ref_t<jbyteArray> make_byte_array(const char*, size_t);
    local_ref_t<jbyteArray> make_byte_array(const std::string&);
    std::string to_string(jbyteArray);

    local_ref_t<jobject> make_direct_byte_buffer(char*, size_t);

    local_ref_t<jclass> find_class(const char*);

    class constructor {
    public:
      constructor()
        : method_() {}

      template <class U>
      constructor(const U& clazz, const char* signature)
        : method_(check(get_env()->GetMethodID(unref(clazz), "<init>", signature))) {}

      template <class U, class... U_args>
      local_ref_t<jobject> operator()(const U& clazz, const U_args&... args) const {
        local_ref_t<jobject> result = make_local_ref(get_env()->NewObject(unref(clazz), method_, unref(args)...));
        check();
        return result;
      }

    private:
      jmethodID method_;
    };

    template <class T, class = void>
    struct method_impl;

    template <class T>
    struct method_impl<T, enable_if_jobject_t<T>> {
      using type = local_ref_t<T>;

      template <class U, class... U_args>
      static type call(const U& object, jmethodID method, const U_args&... args) {
        type result = make_local_ref(reinterpret_cast<T>(get_env()->CallObjectMethod(unref(object), method, unref(args)...)));
        check();
        return result;
      }
    };

    template <class T>
    struct method_impl<T, enable_if_t<std::is_void<T>::value>> {
      using type = void;

      template <class U, class... U_args>
      static type call(const U& object, jmethodID method, const U_args&... args) {
        get_env()->CallVoidMethod(unref(object), method, unref(args)...);
        check();
      }
    };

    template <class T>
    struct method_impl<T, enable_if_t<std::is_same<T, jint>::value>> {
      using type = T;

      template <class U, class... U_args>
      static type call(const U& object, jmethodID method, const U_args&... args) {
        type result = get_env()->CallIntMethod(unref(object), method, unref(args)...);
        check();
        return result;
      }
    };

    template <class T>
    class method {
    public:
      method()
        : method_() {}

      template <class U>
      method(const U& clazz, const char* name, const char* signature)
        : method_(check(get_env()->GetMethodID(unref(clazz), name, signature))) {}

      template <class U, class... U_args>
      typename method_impl<T>::type operator()(const U& object, const U_args&... args) const {
        return method_impl<T>::call(unref(object), method_, unref(args)...);
      }

    private:
      jmethodID method_;
    };
  }
}

#endif
