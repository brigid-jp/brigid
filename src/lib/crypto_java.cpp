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

namespace brigid {
  namespace {
    using namespace java;

    class vtable : private noncopyable {
    public:
      vtable(const char* name)
        : clazz(make_global_ref(find_class(name))),
          construct(clazz, "([B[B)V"),
          update(clazz, "update", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;Z)I") {}

      global_ref_t<jclass> clazz;
      constructor_method construct;
      method<jint> update;
    };

    class aes_cryptor_impl : public cryptor, private noncopyable {
    public:
      aes_cryptor_impl(const char* name, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : vt_(name),
          instance_(make_global_ref(vt_.construct(
              vt_.clazz,
              make_byte_array(key_data, key_size),
              make_byte_array(iv_data, iv_size)))) {}

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        return vt_.update(
            instance_,
            make_direct_byte_buffer(const_cast<char*>(in_data), in_size),
            make_direct_byte_buffer(out_data, out_size),
            to_boolean(padding));
      }

    private:
      vtable vt_;
      global_ref_t<jobject> instance_;
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
