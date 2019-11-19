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

      jni_local_ref_t<jclass> Throwable = jni_make_local_ref(env->FindClass("java/lang/Throwable"));
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        Throwable.reset();
      }
      if (!Throwable) {
        throw std::runtime_error(what);
      }

      jmethodID Throwable_toString = env->GetMethodID(Throwable.get(), "toString", "()Ljava/lang/String;");
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        Throwable_toString = nullptr;
      }
      if (!Throwable_toString) {
        throw std::runtime_error(what);
      }

      jni_local_ref_t<jstring> message = jni_make_local_ref(reinterpret_cast<jstring>(env->CallObjectMethod(exception.get(), Throwable_toString)));
      if (env->ExceptionCheck()) {
        env->ExceptionClear();
        message.reset();
      }
      if (!message) {
        throw std::runtime_error(what);
      }

      std::vector<char> buffer(env->GetStringUTFLength(message.get()));
      env->GetStringUTFRegion(message.get(), 0, env->GetStringLength(message.get()), buffer.data());
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

    class encryptor_java_impl : public encryptor_impl {
    public:
      encryptor_java_impl(const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : Encryptor_(jni_make_global_ref<jclass>()),
          Encryptor_instance_(jni_make_global_ref<jobject>()),
          Encryptor_update_(nullptr) {
        JNIEnv* env = jni_env();

        jni_local_ref_t<jclass> Encryptor = jni_make_local_ref(jni_check(env->FindClass("jp/brigid/Encryptor")));
        Encryptor_ = jni_make_global_ref(jni_check(reinterpret_cast<jclass>(env->NewGlobalRef(Encryptor.get()))));
        Encryptor_update_ = jni_check(env->GetMethodID(Encryptor_.get(), "update", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;Z)I"));

        jmethodID Encryptor_init = jni_check(env->GetMethodID(Encryptor_.get(), "<init>", "([B[B)V"));

        jni_local_ref_t<jbyteArray> key = jni_make_local_ref(jni_check(env->NewByteArray(key_size)));
        env->SetByteArrayRegion(key.get(), 0, key_size, reinterpret_cast<const jbyte*>(key_data));
        jni_check();

        jni_local_ref_t<jbyteArray> iv = jni_make_local_ref(jni_check(env->NewByteArray(iv_size)));
        env->SetByteArrayRegion(iv.get(), 0, iv_size, reinterpret_cast<const jbyte*>(iv_data));
        jni_check();

        jni_local_ref_t<jobject> instance = jni_make_local_ref(jni_check(env->NewObject(Encryptor_.get(), Encryptor_init, key.get(), iv.get())));
        Encryptor_instance_ = jni_make_global_ref(jni_check(env->NewGlobalRef(instance.get())));
      }

      virtual size_t block_bytes() const {
        return 16;
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        JNIEnv* env = jni_env();

        jni_local_ref_t<jobject> in = jni_make_local_ref(jni_check(env->NewDirectByteBuffer(const_cast<char*>(in_data), in_size)));
        jni_local_ref_t<jobject> out = jni_make_local_ref(jni_check(env->NewDirectByteBuffer(out_data, out_size)));

        jint result = env->CallIntMethod(Encryptor_instance_.get(), Encryptor_update_, in.get(), out.get(), padding ? JNI_TRUE : JNI_FALSE);
        jni_check();

        return result;
      }

    private:
      jni_global_ref_t<jclass> Encryptor_;
      jni_global_ref_t<jobject> Encryptor_instance_;
      jmethodID Encryptor_update_;
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-256-cbc") {
      if (key_size != 32) {
        throw std::runtime_error("invalid key size");
      }
      if (iv_size != 16) {
        throw std::runtime_error("invalid iv size");
      }
      return std::unique_ptr<encryptor_impl>(new encryptor_java_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
