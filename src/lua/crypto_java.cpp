// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common_java.hpp"
#include "crypto.hpp"
#include "error.hpp"
#include "noncopyable.hpp"

#include <jni.h>

#include <stddef.h>
#include <memory>
#include <mutex>
#include <vector>

namespace brigid {
  namespace {
    jclass aes_cryptor_clazz;

    class aes_cryptor_vtable : private noncopyable {
    public:
      aes_cryptor_vtable()
        : constructor(aes_cryptor_clazz, "(Z[B[B)V"),
          update(aes_cryptor_clazz, "update", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;Z)I") {}

      constructor_method constructor;
      method<jint> update;
    };

    class aes_cryptor_impl : public cryptor, private noncopyable {
    public:
      aes_cryptor_impl(bool encrypt, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, size_t buffer_size)
        : instance_(make_global_ref(vt_.constructor(
              aes_cryptor_clazz,
              to_boolean(encrypt),
              make_byte_array(key_data, key_size),
              make_byte_array(iv_data, iv_size)))),
          buffer_size_(buffer_size) {}

      virtual size_t calculate_buffer_size(size_t in_size) const {
        return in_size + buffer_size_;
      };

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        return vt_.update(
            instance_,
            make_direct_byte_buffer(const_cast<char*>(in_data), in_size),
            make_direct_byte_buffer(out_data, out_size),
            to_boolean(padding));
      }

    private:
      aes_cryptor_vtable vt_;
      global_ref_t<jobject> instance_;
      size_t buffer_size_;
    };

    jclass hasher_clazz;

    class hasher_vtable : private noncopyable {
    public:
      hasher_vtable()
        : constructor(hasher_clazz, "([B)V"),
          update(hasher_clazz, "update", "(Ljava/nio/ByteBuffer;)V"),
          digest(hasher_clazz, "digest", "()[B") {}

      constructor_method constructor;
      method<void> update;
      method<jbyteArray> digest;
    };

    class hasher_impl : public hasher, private noncopyable {
    public:
      hasher_impl(const char* algorithm)
        : instance_(make_global_ref(vt_.constructor(
              hasher_clazz,
              make_byte_array(algorithm)))) {}

      virtual void update(const char* data, size_t size) {
        vt_.update(instance_, make_direct_byte_buffer(const_cast<char*>(data), size));
      }

      virtual std::vector<char> digest() {
        local_ref_t<jbyteArray> result = vt_.digest(instance_);
        std::vector<char> buffer(get_array_length(result));
        get_byte_array_region(result, 0, buffer.size(), buffer.data());
        return buffer;
      }

    private:
      hasher_vtable vt_;
      global_ref_t<jobject> instance_;
    };

    std::mutex open_cryptor_mutex;
    std::mutex open_hasher_mutex;
  }

  crypto_initializer::crypto_initializer() {}
  crypto_initializer::~crypto_initializer() {}

  void open_cryptor() {
    std::lock_guard<std::mutex> lock(open_cryptor_mutex);
    if (!aes_cryptor_clazz) {
      aes_cryptor_clazz = make_global_ref(find_class("jp/brigid/AESCryptor")).release();
    }
  }

  void open_hasher() {
    std::lock_guard<std::mutex> lock(open_hasher_mutex);
    if (!hasher_clazz) {
      hasher_clazz = make_global_ref(find_class("jp/brigid/Hasher")).release();
    }
  }

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        return std::unique_ptr<cryptor>(new aes_cryptor_impl(true, key_data, key_size, iv_data, iv_size, 16));
    }
    throw BRIGID_LOGIC_ERROR("unsupported cipher");
  }

  std::unique_ptr<cryptor> make_decryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        return std::unique_ptr<cryptor>(new aes_cryptor_impl(false, key_data, key_size, iv_data, iv_size, 0));
    }
    throw BRIGID_LOGIC_ERROR("unsupported cipher");
  }

  std::unique_ptr<hasher> make_hasher(crypto_hash hash) {
    switch (hash) {
      case crypto_hash::sha1:
        return std::unique_ptr<hasher>(new hasher_impl("SHA-1"));
      case crypto_hash::sha256:
        return std::unique_ptr<hasher>(new hasher_impl("SHA-256"));
      case crypto_hash::sha512:
        return std::unique_ptr<hasher>(new hasher_impl("SHA-512"));
    }
    throw BRIGID_LOGIC_ERROR("unsupported hash");
  }
}
