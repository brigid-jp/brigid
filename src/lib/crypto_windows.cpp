// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include <brigid/type_traits.hpp>
#include "common_windows.hpp"

#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>

#include <stddef.h>
#include <string.h>
#include <string>
#include <memory>
#include <vector>

namespace brigid {
  namespace {
    void check(NTSTATUS code) {
      if (!BCRYPT_SUCCESS(code)) {
        std::string message;
        if (get_error_message("ntdll.dll", code, message)) {
          throw BRIGID_ERROR(message, make_error_code("bcrypt error", code));
        } else {
          throw BRIGID_ERROR(make_error_code("bcrypt error", code));
        }
      }
    }

    void close_algorithm_provider(BCRYPT_ALG_HANDLE handle) {
      BCryptCloseAlgorithmProvider(handle, 0);
    }

    using alg_handle_t = std::unique_ptr<remove_pointer_t<BCRYPT_ALG_HANDLE>, decltype(&close_algorithm_provider)>;

    alg_handle_t make_alg_handle(BCRYPT_ALG_HANDLE handle = nullptr) {
      return alg_handle_t(handle, &close_algorithm_provider);
    }

    using key_handle_t = std::unique_ptr<remove_pointer_t<BCRYPT_KEY_HANDLE>, decltype(&BCryptDestroyKey)>;

    key_handle_t make_key_handle(BCRYPT_KEY_HANDLE handle = nullptr) {
      return key_handle_t(handle, &BCryptDestroyKey);
    }

    class aes_cryptor_impl : public cryptor {
    public:
      aes_cryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : alg_(make_alg_handle()),
          key_(make_key_handle()),
          iv_(iv_size) {
        BCRYPT_ALG_HANDLE alg = nullptr;
        check(BCryptOpenAlgorithmProvider(
            &alg,
            BCRYPT_AES_ALGORITHM,
            nullptr,
            0));
        alg_ = make_alg_handle(alg);

        check(BCryptSetProperty(
            alg_.get(),
            BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(BCRYPT_CHAIN_MODE_CBC),
            sizeof(BCRYPT_CHAIN_MODE_CBC),
            0));

        DWORD size = 0;
        DWORD result = 0;
        check(BCryptGetProperty(
            alg_.get(),
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&size),
            sizeof(size),
            &result,
            0));
        buffer_.resize(size);

        BCRYPT_KEY_HANDLE key = nullptr;
        check(BCryptGenerateSymmetricKey(
            alg_.get(),
            &key,
            buffer_.data(),
            static_cast<ULONG>(buffer_.size()),
            reinterpret_cast<PUCHAR>(const_cast<char*>(key_data)),
            static_cast<ULONG>(key_size),
            0));
        key_ = make_key_handle(key);

        memmove(iv_.data(), iv_data, iv_size);
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        return update_impl(key_.get(), iv_, in_data, in_size, out_data, out_size, padding);
      }

      virtual size_t update_impl(BCRYPT_KEY_HANDLE, std::vector<UCHAR>&, const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) = 0;

    private:
      alg_handle_t alg_;
      std::vector<UCHAR> buffer_;
      key_handle_t key_;
      std::vector<UCHAR> iv_;
    };

    class aes_encryptor_impl : public aes_cryptor_impl, private noncopyable {
    public:
      aes_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : aes_cryptor_impl(key_data, key_size, iv_data, iv_size) {}

      virtual size_t calculate_buffer_size(size_t in_size) const {
        return in_size + 16;
      };

      virtual size_t update_impl(BCRYPT_KEY_HANDLE key, std::vector<UCHAR>& iv, const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        ULONG result = 0;
        check(BCryptEncrypt(
            key,
            reinterpret_cast<PUCHAR>(const_cast<char*>(in_data)),
            static_cast<ULONG>(in_size),
            nullptr,
            iv.data(),
            static_cast<ULONG>(iv.size()),
            reinterpret_cast<PUCHAR>(out_data),
            static_cast<ULONG>(out_size),
            &result,
            padding ? BCRYPT_BLOCK_PADDING : 0));
        return result;
      }
    };

    class aes_decryptor_impl : public aes_cryptor_impl, private noncopyable {
    public:
      aes_decryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : aes_cryptor_impl(key_data, key_size, iv_data, iv_size) {}

      virtual size_t calculate_buffer_size(size_t in_size) const {
        return in_size;
      };

      virtual size_t update_impl(BCRYPT_KEY_HANDLE key, std::vector<UCHAR>& iv, const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        ULONG result = 0;
        check(BCryptDecrypt(
            key,
            reinterpret_cast<PUCHAR>(const_cast<char*>(in_data)),
            static_cast<ULONG>(in_size),
            nullptr,
            iv.data(),
            static_cast<ULONG>(iv.size()),
            reinterpret_cast<PUCHAR>(out_data),
            static_cast<ULONG>(out_size),
            &result,
            padding ? BCRYPT_BLOCK_PADDING : 0));
        return result;
      }
    };
  }

  crypto_initializer::crypto_initializer() {}
  crypto_initializer::~crypto_initializer() {}

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        if (iv_size != 16) {
          throw BRIGID_ERROR("invalid initialization vector size");
        }
        return std::unique_ptr<cryptor>(new aes_encryptor_impl(key_data, key_size, iv_data, iv_size));
    }
    throw BRIGID_ERROR("unsupported cipher");
  }

  std::unique_ptr<cryptor> make_decryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        if (iv_size != 16) {
          throw BRIGID_ERROR("invalid initialization vector size");
        }
        return std::unique_ptr<cryptor>(new aes_decryptor_impl(key_data, key_size, iv_data, iv_size));
    }
    throw BRIGID_ERROR("unsupported cipher");
  }
}
