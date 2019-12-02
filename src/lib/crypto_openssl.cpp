// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/noncopyable.hpp>
#include "crypto_impl.hpp"
#include "error.hpp"

#include <openssl/err.h>
#include <openssl/evp.h>

#include <vector>

namespace brigid {
  namespace {
    template <class T>
    T check(T result) {
      if (!result) {
        unsigned long code = ERR_get_error();
        std::vector<char> buffer(256);
        ERR_error_string_n(code, buffer.data(), buffer.size());
        throw BRIGID_ERROR(buffer.data(), make_error_code("OpenSSL error code", code));
      }
      return result;
    }

    using cipher_ctx_t = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

    cipher_ctx_t make_cipher_ctx(EVP_CIPHER_CTX* ctx) {
      return cipher_ctx_t(ctx, &EVP_CIPHER_CTX_free);
    }

    class aes_encryptor_impl : public cryptor, private noncopyable {
    public:
      aes_encryptor_impl(const EVP_CIPHER* cipher, const char* key_data, const char* iv_data)
        : ctx_(make_cipher_ctx(check(EVP_CIPHER_CTX_new()))) {
        check(EVP_EncryptInit_ex(ctx_.get(), cipher, nullptr, reinterpret_cast<const unsigned char*>(key_data), reinterpret_cast<const unsigned char*>(iv_data)));
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        int size1 = out_size;
        int size2 = 0;
        check(EVP_EncryptUpdate(ctx_.get(), reinterpret_cast<unsigned char*>(out_data), &size1, reinterpret_cast<const unsigned char*>(in_data), in_size));
        if (padding) {
          size2 = out_size - size1;
          check(EVP_EncryptFinal_ex(ctx_.get(), reinterpret_cast<unsigned char*>(out_data + size1), &size2));
        }
        return size1 + size2;
      }

    private:
      cipher_ctx_t ctx_;
    };

    class aes_decryptor_impl : public cryptor, private noncopyable {
    public:
      aes_decryptor_impl(const EVP_CIPHER* cipher, const char* key_data, const char* iv_data)
        : ctx_(make_cipher_ctx(check(EVP_CIPHER_CTX_new()))) {
        check(EVP_DecryptInit_ex(ctx_.get(), cipher, nullptr, reinterpret_cast<const unsigned char*>(key_data), reinterpret_cast<const unsigned char*>(iv_data)));
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        int size1 = out_size;
        int size2 = 0;
        check(EVP_DecryptUpdate(ctx_.get(), reinterpret_cast<unsigned char*>(out_data), &size1, reinterpret_cast<const unsigned char*>(in_data), in_size));
        if (padding) {
          size2 = out_size - size1;
          check(EVP_DecryptFinal_ex(ctx_.get(), reinterpret_cast<unsigned char*>(out_data + size1), &size2));
        }
        return size1 + size2;
      }

    private:
      cipher_ctx_t ctx_;
    };
  }

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check_cipher(cipher, key_size, iv_size);
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
        return std::unique_ptr<cryptor>(new aes_encryptor_impl(EVP_aes_128_cbc(), key_data, iv_data));
      case crypto_cipher::aes_192_cbc:
        return std::unique_ptr<cryptor>(new aes_encryptor_impl(EVP_aes_192_cbc(), key_data, iv_data));
      case crypto_cipher::aes_256_cbc:
        return std::unique_ptr<cryptor>(new aes_encryptor_impl(EVP_aes_256_cbc(), key_data, iv_data));
      default:
        throw BRIGID_ERROR("unsupported cipher");
    }
  }

  std::unique_ptr<cryptor> make_decryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check_cipher(cipher, key_size, iv_size);
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
        return std::unique_ptr<cryptor>(new aes_decryptor_impl(EVP_aes_128_cbc(), key_data, iv_data));
      case crypto_cipher::aes_192_cbc:
        return std::unique_ptr<cryptor>(new aes_decryptor_impl(EVP_aes_192_cbc(), key_data, iv_data));
      case crypto_cipher::aes_256_cbc:
        return std::unique_ptr<cryptor>(new aes_decryptor_impl(EVP_aes_256_cbc(), key_data, iv_data));
      default:
        throw BRIGID_ERROR("unsupported cipher");
    }
  }
}
