// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/type_traits.hpp>

#include <jni.h>

extern "C" void* SDL_AndroidGetJNIEnv();

#include <memory>
#include <stdexcept>
#include <vector>

namespace brigid {
  namespace {
    JNIEnv* jni_env() {
      return static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    }

    void jni_delete_local_ref(jobject that) {
      jni_env()->DeleteLocalRef(that);
    }

    template <class T>
    using jni_local_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&jni_delete_local_ref)>;

    template <class T>
    jni_local_ref_t<T> jni_make_local_ref(T that = nullptr) {
      return jni_local_ref_t<T>(that, &jni_delete_local_ref);
    }

    void jni_delete_global_ref(jobject that) {
      jni_env()->DeleteGlobalRef(that);
    }

    template <class T>
    using jni_global_ref_t = std::unique_ptr<remove_pointer_t<T>, decltype(&jni_delete_global_ref)>;

    template <class T>
    jni_global_ref_t<T> jni_make_global_ref(T that = nullptr) {
      return jni_global_ref_t<T>(that, &jni_delete_global_ref);
    }

    void jni_check() {
      static const char* what = "crypto_java exception";

      JNIEnv* env = jni_env();
      if (!env->ExceptionCheck()) {
        return;
      }
      jni_local_ref_t<jthrowable> exception = jni_make_local_ref(env->ExceptionOccurred());
      env->ExceptionClear();

      jni_local_ref_t<jclass> klass = jni_make_local_ref(env->FindClass("java/lang/Throwable"));
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

      jni_local_ref_t<jstring> result = jni_make_local_ref(reinterpret_cast<jstring>(env->CallObjectMethod(exception.get(), method)));
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

    template <class T>
    T* jni_check(T* that) {
      jni_check();
      if (!that) {
        throw std::runtime_error("crypto_java exception");
      }
      return that;
    }

    jboolean jni_boolean(bool value) {
      if (value) {
        return JNI_TRUE;
      } else {
        return JNI_FALSE;
      }
    }

    class aes_encryptor_impl : public encryptor_impl {
    public:
      aes_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : klass_(jni_make_global_ref<jclass>()),
          instance_(jni_make_global_ref<jobject>()),
          method_(nullptr) {
        JNIEnv* env = jni_env();

        jni_local_ref_t<jclass> klass = jni_make_local_ref(jni_check(env->FindClass("jp/brigid/AESEncryptor")));
        klass_ = jni_make_global_ref(jni_check(reinterpret_cast<jclass>(env->NewGlobalRef(klass.get()))));

        // new AESEncryptor(key, iv)
        {
          jmethodID method = jni_check(env->GetMethodID(klass.get(), "<init>", "([B[B)V"));

          jni_local_ref_t<jbyteArray> key = jni_make_local_ref(jni_check(env->NewByteArray(key_size)));
          env->SetByteArrayRegion(key.get(), 0, key_size, reinterpret_cast<const jbyte*>(key_data));
          jni_check();

          jni_local_ref_t<jbyteArray> iv = jni_make_local_ref(jni_check(env->NewByteArray(iv_size)));
          env->SetByteArrayRegion(iv.get(), 0, iv_size, reinterpret_cast<const jbyte*>(iv_data));
          jni_check();

          jni_local_ref_t<jobject> instance = jni_make_local_ref(jni_check(env->NewObject(klass.get(), method, key.get(), iv.get())));
          instance_ = jni_make_global_ref(jni_check(env->NewGlobalRef(instance.get())));
        }

        method_ = jni_check(env->GetMethodID(klass.get(), "update", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;Z)I"));
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        JNIEnv* env = jni_env();

        jni_local_ref_t<jobject> in = jni_make_local_ref(jni_check(env->NewDirectByteBuffer(const_cast<char*>(in_data), in_size)));

        jni_local_ref_t<jobject> out = jni_make_local_ref(jni_check(env->NewDirectByteBuffer(out_data, out_size)));

        jint result = env->CallIntMethod(instance_.get(), method_, in.get(), out.get(), jni_boolean(padding));
        jni_check();

        return result;
      }

    private:
      jni_global_ref_t<jclass> klass_;
      jni_global_ref_t<jobject> instance_;
      jmethodID method_;
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new aes_encryptor_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }

  std::unique_ptr<decryptor_impl> make_decryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      // return std::unique_ptr<decryptor_impl>(new aes_decryptor_impl(key_data, key_size, iv_data, iv_size));
      throw std::runtime_error("unsupported cipher");
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
