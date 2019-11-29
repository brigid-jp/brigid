// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include "crypto_impl.hpp"
#include "java.hpp"
#include "type_traits.hpp"

#include <jni.h>

#include <memory>
#include <stdexcept>
#include <vector>

namespace brigid {
  namespace {
    class aes_cryptor_impl : public cryptor {
    public:
      aes_cryptor_impl(const char* name, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : klass_(java_make_global_ref<jclass>()),
          instance_(java_make_global_ref<jobject>()),
          method_(nullptr) {
        JNIEnv* env = java_env();

        java_local_ref_t<jclass> klass = java_make_local_ref(java_check(env->FindClass(name)));
        klass_ = java_make_global_ref(java_check(reinterpret_cast<jclass>(env->NewGlobalRef(klass.get()))));

        {
          jmethodID method = java_check(env->GetMethodID(klass.get(), "<init>", "([B[B)V"));

          java_local_ref_t<jbyteArray> key = java_make_local_ref(java_check(env->NewByteArray(key_size)));
          env->SetByteArrayRegion(key.get(), 0, key_size, reinterpret_cast<const jbyte*>(key_data));
          java_check();

          java_local_ref_t<jbyteArray> iv = java_make_local_ref(java_check(env->NewByteArray(iv_size)));
          env->SetByteArrayRegion(iv.get(), 0, iv_size, reinterpret_cast<const jbyte*>(iv_data));
          java_check();

          java_local_ref_t<jobject> instance = java_make_local_ref(java_check(env->NewObject(klass.get(), method, key.get(), iv.get())));
          instance_ = java_make_global_ref(java_check(env->NewGlobalRef(instance.get())));
        }

        method_ = java_check(env->GetMethodID(klass.get(), "update", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;Z)I"));
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        JNIEnv* env = java_env();

        java_local_ref_t<jobject> in = java_make_local_ref(java_check(env->NewDirectByteBuffer(const_cast<char*>(in_data), in_size)));

        java_local_ref_t<jobject> out = java_make_local_ref(java_check(env->NewDirectByteBuffer(out_data, out_size)));

        jint result = env->CallIntMethod(instance_.get(), method_, in.get(), out.get(), java_boolean(padding));
        java_check();

        return result;
      }

    private:
      java_global_ref_t<jclass> klass_;
      java_global_ref_t<jobject> instance_;
      jmethodID method_;
    };
  }

  std::unique_ptr<cryptor> make_encryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check_cipher(cipher, key_size, iv_size);
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      return std::unique_ptr<cryptor>(new aes_cryptor_impl("jp/brigid/AESEncryptor", key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }

  std::unique_ptr<cryptor> make_decryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check_cipher(cipher, key_size, iv_size);
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      return std::unique_ptr<cryptor>(new aes_cryptor_impl("jp/brigid/AESDecryptor", key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
