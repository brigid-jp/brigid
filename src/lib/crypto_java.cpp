// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/noncopyable.hpp>
#include "error.hpp"
#include "util_java.hpp"

#include <jni.h>

#include <stddef.h>
#include <memory>
#include <string>

namespace brigid {
  namespace {
    using namespace java;

    class aes_cryptor_impl : public cryptor, private noncopyable {
    public:
      aes_cryptor_impl(const char* name, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : klass_(make_global_ref<jclass>()),
          instance_(make_global_ref<jobject>()),
          method_() {
        JNIEnv* env = get_env();

        local_ref_t<jclass> klass = make_local_ref(check(env->FindClass(name)));
        klass_ = make_global_ref(check(reinterpret_cast<jclass>(env->NewGlobalRef(klass.get()))));

        {
          jmethodID method = check(env->GetMethodID(klass.get(), "<init>", "([B[B)V"));

          local_ref_t<jbyteArray> key = make_local_ref(check(env->NewByteArray(key_size)));
          env->SetByteArrayRegion(key.get(), 0, key_size, reinterpret_cast<const jbyte*>(key_data));
          check();

          local_ref_t<jbyteArray> iv = make_local_ref(check(env->NewByteArray(iv_size)));
          env->SetByteArrayRegion(iv.get(), 0, iv_size, reinterpret_cast<const jbyte*>(iv_data));
          check();

          local_ref_t<jobject> instance = make_local_ref(check(env->NewObject(klass.get(), method, key.get(), iv.get())));
          instance_ = make_global_ref(check(env->NewGlobalRef(instance.get())));
        }

        method_ = check(env->GetMethodID(klass.get(), "update", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;Z)I"));
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        JNIEnv* env = get_env();

        local_ref_t<jobject> in = make_local_ref(check(env->NewDirectByteBuffer(const_cast<char*>(in_data), in_size)));

        local_ref_t<jobject> out = make_local_ref(check(env->NewDirectByteBuffer(out_data, out_size)));

        jint result = env->CallIntMethod(instance_.get(), method_, in.get(), out.get(), to_boolean(padding));
        check();

        return result;
      }

    private:
      global_ref_t<jclass> klass_;
      global_ref_t<jobject> instance_;
      jmethodID method_;
    };
  }

  crypto_initializer::crypto_initializer() {}
  crypto_initializer::~crypto_initializer() {}

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        return std::unique_ptr<cryptor>(new aes_cryptor_impl("jp/brigid/AESEncryptor", key_data, key_size, iv_data, iv_size));
    }
    throw BRIGID_ERROR("unsupported cipher");
  }

  std::unique_ptr<cryptor> make_decryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        return std::unique_ptr<cryptor>(new aes_cryptor_impl("jp/brigid/AESDecryptor", key_data, key_size, iv_data, iv_size));
    }
    throw BRIGID_ERROR("unsupported cipher");
  }
}
