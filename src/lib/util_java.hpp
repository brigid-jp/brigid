// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_UTIL_JAVA_HPP
#define BRIGID_UTIL_JAVA_HPP

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "type_traits.hpp"

#include <jni.h>

#include <stddef.h>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace java {
    template <bool T_condition, class T = void>
    using enable_if_t = typename std::enable_if<T_condition, T>::type;

    template <class T>
    using is_jobject_t = std::is_base_of<remove_pointer_t<jobject>, remove_pointer_t<T>>;

    JNIEnv* get_env();

    void check_impl(bool);

    void check();

    template <class T>
    inline T* check(T* result) {
      check_impl(!result);
      return result;
    }

    struct delete_local_ref {
      void operator()(jobject) const;
    };

    template <class T>
    using local_ref_t = std::unique_ptr<remove_pointer_t<T>, delete_local_ref>;

    template <class>
    struct is_local_ref_t : std::false_type {};

    template <class T>
    struct is_local_ref_t<std::unique_ptr<T, delete_local_ref>> : std::true_type {};

    template <class T>
    inline local_ref_t<T> make_local_ref(T object, enable_if_t<is_jobject_t<T>::value>* = nullptr) {
      return local_ref_t<T>(object, delete_local_ref());
    }

    struct delete_global_ref {
      void operator()(jobject) const;
    };

    template <class T>
    using global_ref_t = std::unique_ptr<remove_pointer_t<T>, delete_global_ref>;

    template <class>
    struct is_global_ref_t : std::false_type {};

    template <class T>
    struct is_global_ref_t<std::unique_ptr<T, delete_global_ref>> : std::true_type {};

    template <class T>
    inline global_ref_t<T> make_global_ref(enable_if_t<is_jobject_t<T>::value>* = nullptr) {
      return global_ref_t<T>(nullptr, delete_global_ref());
    }

    template <class T>
    inline global_ref_t<typename T::pointer> make_global_ref(const T& source, enable_if_t<(is_local_ref_t<T>::value && is_jobject_t<typename T::pointer>::value)>* = nullptr) {
      global_ref_t<typename T::pointer> result(reinterpret_cast<typename T::pointer>(get_env()->NewGlobalRef(source.get())), delete_global_ref());
      check(result.get());
      return result;
    }

    template <class T, class = void>
    struct unref_impl {
      using type = T;

      static type unref(const T& source) {
        return source;
      }
    };

    template <class T>
    struct unref_impl<T, enable_if_t<((is_local_ref_t<T>::value || is_global_ref_t<T>::value) && is_jobject_t<typename T::pointer>::value)>> {
      using type = typename T::pointer;

      static type unref(const T& source) {
        return source.get();
      }
    };

    template <class T>
    using unref_t = typename unref_impl<T>::type;

    template <class T>
    inline unref_t<T> unref(const T& source) {
      return unref_impl<T>::unref(source);
    }

    local_ref_t<jbyteArray> make_byte_array(size_t);
    local_ref_t<jbyteArray> make_byte_array(const char*, size_t);
    local_ref_t<jbyteArray> make_byte_array(const std::string&);
    local_ref_t<jobject> make_direct_byte_buffer(void*, size_t);

    jboolean to_boolean(bool);

    template <class T>
    inline size_t get_array_length(const T& source) {
      return get_env()->GetArrayLength(unref(source));
    }

    template <class T>
    inline void get_byte_array_region(const T& source, size_t position, size_t size, char* target, enable_if_t<std::is_same<unref_t<T>, jbyteArray>::value>* = nullptr) {
      get_env()->GetByteArrayRegion(unref(source), position, size, reinterpret_cast<jbyte*>(target));
      check();
    }

    template <class T>
    inline std::string get_byte_array_region(const T& source, size_t position, size_t size, enable_if_t<std::is_same<unref_t<T>, jbyteArray>::value>* = nullptr) {
      std::vector<char> buffer(size);
      get_byte_array_region(source, position, size, buffer.data());
      return std::string(buffer.data(), buffer.size());
    }

    template <class T>
    inline std::string get_byte_array_region(const T& source, size_t position = 0, enable_if_t<std::is_same<unref_t<T>, jbyteArray>::value>* = nullptr) {
      size_t size = get_array_length(source);
      if (size < position) {
        throw BRIGID_ERROR("invalid position");
      }
      return get_byte_array_region(source, position, size - position);
    }

    template <class T>
    inline void set_byte_array_region(const T& target, size_t position, size_t size, const char* source, enable_if_t<std::is_same<unref_t<T>, jbyteArray>::value>* = nullptr) {
      get_env()->SetByteArrayRegion(unref(target), position, size, reinterpret_cast<const jbyte*>(source));
      check();
    }

    local_ref_t<jclass> find_class(const char*);

    class constructor_method : private noncopyable {
    public:
      using result_type = local_ref_t<jobject>;

      template <class U>
      constructor_method(const U& clazz, const char* signature)
        : method_(check(get_env()->GetMethodID(unref(clazz), "<init>", signature))) {}

      template <class U, class... U_args>
      result_type operator()(const U& clazz, const U_args&... args) const {
        result_type result = make_local_ref(get_env()->NewObject(unref(clazz), method_, unref(args)...));
        check(result.get());
        return result;
      }

    private:
      jmethodID method_;
    };

    template <class T, class = void>
    struct method_impl;

    template <class T>
    struct method_impl<T, enable_if_t<is_jobject_t<T>::value>> {
      using result_type = local_ref_t<T>;

      template <class U, class... U_args>
      static result_type call(const U& object, jmethodID method, const U_args&... args) {
        result_type result = make_local_ref(reinterpret_cast<T>(get_env()->CallObjectMethod(unref(object), method, unref(args)...)));
        check();
        return result;
      }
    };

    template <class T>
    struct method_impl<T, enable_if_t<std::is_void<T>::value>> {
      using result_type = T;

      template <class U, class... U_args>
      static result_type call(const U& object, jmethodID method, const U_args&... args) {
        get_env()->CallVoidMethod(unref(object), method, unref(args)...);
        check();
      }
    };

    template <class T>
    struct method_impl<T, enable_if_t<std::is_same<T, jint>::value>> {
      using result_type = T;

      template <class U, class... U_args>
      static result_type call(const U& object, jmethodID method, const U_args&... args) {
        result_type result = get_env()->CallIntMethod(unref(object), method, unref(args)...);
        check();
        return result;
      }
    };

    template <class T>
    class method : private noncopyable {
    public:
      using result_type = typename method_impl<T>::result_type;

      template <class U>
      method(const U& clazz, const char* name, const char* signature)
        : method_(check(get_env()->GetMethodID(unref(clazz), name, signature))) {}

      template <class U, class... U_args>
      result_type operator()(const U& object, const U_args&... args) const {
        return method_impl<T>::call(unref(object), method_, unref(args)...);
      }

    private:
      jmethodID method_;
    };

    template <class T, class = void>
    struct static_method_impl;

    template <class T>
    struct static_method_impl<T, enable_if_t<std::is_void<T>::value>> {
      using result_type = T;

      template <class U, class... U_args>
      static result_type call(const U& clazz, jmethodID method, const U_args&... args) {
        get_env()->CallStaticVoidMethod(unref(clazz), method, unref(args)...);
        check();
      }
    };

    template <class T>
    class static_method : private noncopyable {
    public:
      using result_type = typename static_method_impl<T>::result_type;

      template <class U>
      static_method(const U& clazz, const char* name, const char* signature)
        : method_(check(get_env()->GetStaticMethodID(unref(clazz), name, signature))) {}

      template <class U, class... U_args>
      result_type operator()(const U& object, const U_args&... args) const {
        return static_method_impl<T>::call(unref(object), method_, unref(args)...);
      }

    private:
      jmethodID method_;
    };
  }
}

#endif
