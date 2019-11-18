// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <jni.h>

extern "C" void* SDL_AndroidGetJNIEnv();

#include <iostream>
#include <memory>
#include <stdexcept>

#include <brigid/type_traits.hpp>

namespace brigid {
  namespace {
    JNIEnv* jni_env() {
      return static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    }

    struct global_ref_deleter {
      JNIEnv* env;

      void operator()(jobject that) const {
        std::cout << "deleter " << env << " " << that << "\n";
        env->DeleteGlobalRef(that);
      }
    };

    template <class T>
    using global_ref_t = std::unique_ptr<remove_pointer_t<T>, global_ref_deleter>;

    template <class T>
    global_ref_t<T> make_global_ref(T that) {
      return global_ref_t<T> { that, global_ref_deleter { jni_env() } };
    }

    struct local_ref_deleter {
      JNIEnv* env;

      void operator()(jobject that) const {
        env->DeleteLocalRef(that);
      }
    };

    template <class T>
    using local_ref_t = std::unique_ptr<remove_pointer_t<T>, local_ref_deleter>;

    template <class T>
    local_ref_t<T> make_local_ref(T that) {
      return local_ref_t<T>(that, local_ref_deleter{ jni_env() });
    }

    struct string_utf_chars_deleter {
      JNIEnv* env;
      jstring self;

      void operator()(const char* that) const {
        env->ReleaseStringUTFChars(self, that);
      }
    };

    using string_utf_chars_t = std::unique_ptr<const char, string_utf_chars_deleter>;

    void jni_check_exception() {
      JNIEnv* env = jni_env();
      if (env->ExceptionCheck()) {
        local_ref_t<jthrowable> e = make_local_ref(env->ExceptionOccurred());
        env->ExceptionClear();

        local_ref_t<jclass> Throwable = make_local_ref(env->FindClass("java/lang/Throwable"));
        if (env->ExceptionCheck()) {
          env->ExceptionClear();
          Throwable.reset();
        }

        jmethodID Throwable_toString = nullptr;
        if (Throwable) {
          Throwable_toString = env->GetMethodID(Throwable.get(), "toString", "()Ljava/lang/String;");
          if (env->ExceptionCheck()) {
            env->ExceptionClear();
            Throwable_toString = nullptr;
          }
        }

        if (Throwable_toString) {
          local_ref_t<jstring> result = make_local_ref(reinterpret_cast<jstring>(env->CallObjectMethod(e.get(), Throwable_toString)));
          if (env->ExceptionCheck()) {
            env->ExceptionClear();
            result.reset();
          }
          if (result) {
            string_utf_chars_t data { env->GetStringUTFChars(result.get(), nullptr), string_utf_chars_deleter { env, result.get() } };
            if (env->ExceptionCheck()) {
              env->ExceptionClear();
              data.reset();
            }
            if (data) {
              jsize size = env->GetStringUTFLength(result.get());
              if (env->ExceptionCheck()) {
                env->ExceptionClear();
              } else {
                throw std::runtime_error("java exception " + std::string(data.get(), data.get() + size));
              }
            }
          }
        }
        throw std::runtime_error("java exception");
      }
    }

    class encryptor_java_impl : public encryptor_impl {
    public:
      encryptor_java_impl(const char* cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : Encryptor_(make_global_ref<jclass>(nullptr)),
          Encryptor_constructor_(nullptr),
          Encryptor_update_(nullptr) {
        JNIEnv* env = jni_env();
        jclass Encryptor = env->FindClass("jp/brigid/Encryptor");
        jni_check_exception();
        if (!Encryptor) {
          throw std::runtime_error("cannot FindClass");
        }
        std::cout << "lEncryptor " << Encryptor << "\n";
        jclass gEncryptor = reinterpret_cast<jclass>(env->NewGlobalRef(Encryptor));
        std::cout << "gEncryptor " << gEncryptor << "\n";
        if (!gEncryptor) {
          throw std::runtime_error("cannot NewGlobalRef");
        }
        Encryptor_ = make_global_ref(gEncryptor);

        Encryptor_constructor_ = env->GetMethodID(Encryptor_.get(), "<init>", "(Ljava/lang/String;[B[B)V");
        jni_check_exception();

        Encryptor_update_ = env->GetMethodID(Encryptor_.get(), "update", "([B[BZ)J");
        jni_check_exception();
      }

      virtual size_t block_bytes() const {
        return 16;
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        return 0;
      }

    private:
      global_ref_t<jclass> Encryptor_;
      jmethodID Encryptor_constructor_;
      jmethodID Encryptor_update_;
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new encryptor_java_impl("AES/CBC/PKCS7Padding", key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
