// Copyright (c) 2021,2022 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "crypto.hpp"
#include "error.hpp"
#include "noncopyable.hpp"

#include <lua.hpp>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <stddef.h>
#include <memory>
#include <vector>

namespace brigid {
  namespace {
    template <class T>
    T check(T result) {
      if (!result) {
        unsigned long code = ERR_get_error();
        std::vector<char> buffer(256);
        ERR_error_string_n(code, buffer.data(), buffer.size());
        throw BRIGID_RUNTIME_ERROR(buffer.data(), make_error_code("openssl error", code));
      }
      return result;
    }

    using cipher_ctx_t = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

    cipher_ctx_t make_cipher_ctx(EVP_CIPHER_CTX* ctx) {
      return cipher_ctx_t(ctx, &EVP_CIPHER_CTX_free);
    }

    class aes_encryptor_impl : public cryptor, private noncopyable {
    public:
      aes_encryptor_impl(const EVP_CIPHER* cipher, const char* key_data, size_t key_size, const char* iv_data)
        : ctx_(make_cipher_ctx(check(EVP_CIPHER_CTX_new()))) {
        check(EVP_EncryptInit_ex(ctx_.get(), cipher, nullptr, reinterpret_cast<const unsigned char*>(key_data), reinterpret_cast<const unsigned char*>(iv_data)));
        check(EVP_CIPHER_CTX_set_key_length(ctx_.get(), key_size));
      }

      virtual size_t calculate_buffer_size(size_t in_size) const {
        return in_size + 16;
      };

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
      aes_decryptor_impl(const EVP_CIPHER* cipher, const char* key_data, size_t key_size, const char* iv_data)
        : ctx_(make_cipher_ctx(check(EVP_CIPHER_CTX_new()))) {
        check(EVP_DecryptInit_ex(ctx_.get(), cipher, nullptr, reinterpret_cast<const unsigned char*>(key_data), reinterpret_cast<const unsigned char*>(iv_data)));
        check(EVP_CIPHER_CTX_set_key_length(ctx_.get(), key_size));
      }

      virtual size_t calculate_buffer_size(size_t in_size) const {
        return in_size;
      };

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

    class sha1_hasher_impl : public hasher, private noncopyable {
    public:
      sha1_hasher_impl()
        : ctx_() {
        check(SHA1_Init(&ctx_));
      }

      virtual const char* get_name() const {
        return "sha1";
      }

      virtual void update(const char* data, size_t size) {
        check(SHA1_Update(&ctx_, data, size));
      }

      virtual std::vector<char> digest() {
        std::vector<char> buffer(SHA_DIGEST_LENGTH);
        check(SHA1_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx_));
        return buffer;
      }

    private:
      SHA_CTX ctx_;
    };

    class sha256_hasher_impl : public hasher, private noncopyable {
    public:
      sha256_hasher_impl()
        : ctx_() {
        check(SHA256_Init(&ctx_));
      }

      virtual const char* get_name() const {
        return "sha256";
      }

      virtual void update(const char* data, size_t size) {
        check(SHA256_Update(&ctx_, data, size));
      }

      virtual std::vector<char> digest() {
        std::vector<char> buffer(SHA256_DIGEST_LENGTH);
        check(SHA256_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx_));
        return buffer;
      }

    private:
      SHA256_CTX ctx_;
    };

    class sha512_hasher_impl : public hasher, private noncopyable {
    public:
      sha512_hasher_impl()
        : ctx_() {
        check(SHA512_Init(&ctx_));
      }

      virtual const char* get_name() const {
        return "sha512";
      }

      virtual void update(const char* data, size_t size) {
        check(SHA512_Update(&ctx_, data, size));
      }

      virtual std::vector<char> digest() {
        std::vector<char> buffer(SHA512_DIGEST_LENGTH);
        check(SHA512_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx_));
        return buffer;
      }

    private:
      SHA512_CTX ctx_;
    };

    int crypto_initializer_count = 0;
  }

  crypto_initializer::crypto_initializer() {
    if (++crypto_initializer_count == 1) {
      ERR_load_crypto_strings();
    }
  }

  crypto_initializer::~crypto_initializer() {
    if (--crypto_initializer_count == 0) {
      ERR_free_strings();
    }
  }

  void open_cryptor() {}
  void open_hasher() {}

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    const EVP_CIPHER* evp_cipher = nullptr;
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
        evp_cipher = EVP_aes_128_cbc();
        break;
      case crypto_cipher::aes_192_cbc:
        evp_cipher = EVP_aes_192_cbc();
        break;
      case crypto_cipher::aes_256_cbc:
        evp_cipher = EVP_aes_256_cbc();
        break;
      default:
        throw BRIGID_LOGIC_ERROR("unsupported cipher");
    }
    if (iv_size != 16) {
      throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
    }
    return std::unique_ptr<cryptor>(new aes_encryptor_impl(evp_cipher, key_data, key_size, iv_data));
  }

  std::unique_ptr<cryptor> make_decryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    const EVP_CIPHER* evp_cipher = nullptr;
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
        evp_cipher = EVP_aes_128_cbc();
        break;
      case crypto_cipher::aes_192_cbc:
        evp_cipher = EVP_aes_192_cbc();
        break;
      case crypto_cipher::aes_256_cbc:
        evp_cipher = EVP_aes_256_cbc();
        break;
      default:
        throw BRIGID_LOGIC_ERROR("unsupported cipher");
    }
    if (iv_size != 16) {
      throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
    }
    return std::unique_ptr<cryptor>(new aes_decryptor_impl(evp_cipher, key_data, key_size, iv_data));
  }

  hasher* new_sha1_hasher(lua_State* L) {
    return new_userdata<sha1_hasher_impl>(L, "brigid.hasher");
  }

  hasher* new_sha256_hasher(lua_State* L) {
    return new_userdata<sha256_hasher_impl>(L, "brigid.hasher");
  }

  hasher* new_sha512_hasher(lua_State* L) {
    return new_userdata<sha512_hasher_impl>(L, "brigid.hasher");
  }
}
