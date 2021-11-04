// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include <brigid/type_traits.hpp>
#include "crypto.hpp"

#include <CommonCrypto/CommonCrypto.h>

#include <stddef.h>
#include <memory>
#include <vector>

namespace brigid {
  namespace {
    const char* get_error_message(CCCryptorStatus code) {
      switch (code) {
        case -4300: return "kCCParamError";
        case -4301: return "kCCBufferTooSmall";
        case -4302: return "kCCMemoryFailure";
        case -4303: return "kCCAlignmentError";
        case -4304: return "kCCDecodeError";
        case -4305: return "kCCUnimplemented";
        case -4306: return "kCCOverflow";
        case -4307: return "kCCRNGFailure";
        case -4308: return "kCCUnspecifiedError";
        case -4309: return "kCCCallSequenceError";
        case -4310: return "kCCKeySizeError";
        case -4311: return "kCCInvalidKey";
      }
      return nullptr;
    }

    void check(CCCryptorStatus code) {
      if (code != kCCSuccess) {
        if (const char* message = get_error_message(code)) {
          throw BRIGID_RUNTIME_ERROR(message, make_error_code("CommonCrypto error", code));
        } else {
          throw BRIGID_RUNTIME_ERROR(make_error_code("CommonCrypto error", code));
        }
      }
    }

    using cryptor_ref_t = std::unique_ptr<remove_pointer_t<CCCryptorRef>, decltype(&CCCryptorRelease)>;

    cryptor_ref_t make_cryptor_ref(CCCryptorRef cryptor = nullptr) {
      return cryptor_ref_t(cryptor, &CCCryptorRelease);
    }

    class aes_cryptor_impl : public cryptor, private noncopyable {
    public:
      aes_cryptor_impl(CCOperation operation, const char* key_data, size_t key_size, const char* iv_data, size_t buffer_size)
        : cryptor_(make_cryptor_ref()),
          buffer_size_(buffer_size) {
        CCCryptorRef cryptor = nullptr;
        check(CCCryptorCreateWithMode(operation, kCCModeCBC, kCCAlgorithmAES, kCCOptionPKCS7Padding, iv_data, key_data, key_size, nullptr, 0, 0, 0, &cryptor));
        cryptor_ = make_cryptor_ref(cryptor);
      }

      virtual size_t calculate_buffer_size(size_t in_size) const {
        return in_size + buffer_size_;
      };

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        size_t size1 = 0;
        size_t size2 = 0;
        check(CCCryptorUpdate(cryptor_.get(), in_data, in_size, out_data, out_size, &size1));
        if (padding) {
          check(CCCryptorFinal(cryptor_.get(), out_data + size1, out_size - size1, &size2));
        }
        return size1 + size2;
      }

    private:
      cryptor_ref_t cryptor_;
      size_t buffer_size_;
    };

    class sha1_hasher_impl : public hasher, private noncopyable {
    public:
      sha1_hasher_impl()
        : ctx_() {
        CC_SHA1_Init(&ctx_);
      }

      virtual void update(const char* data, size_t size) {
        CC_SHA1_Update(&ctx_, data, size);
      }

      virtual std::vector<char> digest() {
        std::vector<char> buffer(CC_SHA1_DIGEST_LENGTH);
        CC_SHA1_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx_);
        return buffer;
      }

    private:
      CC_SHA1_CTX ctx_;
    };

    class sha256_hasher_impl : public hasher, private noncopyable {
    public:
      sha256_hasher_impl()
        : ctx_() {
        CC_SHA256_Init(&ctx_);
      }

      virtual void update(const char* data, size_t size) {
        CC_SHA256_Update(&ctx_, data, size);
      }

      virtual std::vector<char> digest() {
        std::vector<char> buffer(CC_SHA256_DIGEST_LENGTH);
        CC_SHA256_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx_);
        return buffer;
      }

    private:
      CC_SHA256_CTX ctx_;
    };

    class sha512_hasher_impl : public hasher, private noncopyable {
    public:
      sha512_hasher_impl()
        : ctx_() {
        CC_SHA512_Init(&ctx_);
      }

      virtual void update(const char* data, size_t size) {
        CC_SHA512_Update(&ctx_, data, size);
      }

      virtual std::vector<char> digest() {
        std::vector<char> buffer(CC_SHA512_DIGEST_LENGTH);
        CC_SHA512_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx_);
        return buffer;
      }

    private:
      CC_SHA512_CTX ctx_;
    };
  }

  crypto_initializer::crypto_initializer() {}
  crypto_initializer::~crypto_initializer() {}

  void open_cryptor() {}
  void open_hasher() {}

  std::unique_ptr<cryptor> make_encryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        if (iv_size != 16) {
          throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
        }
        return std::unique_ptr<cryptor>(new aes_cryptor_impl(kCCEncrypt, key_data, key_size, iv_data, 16));
    }
    throw BRIGID_LOGIC_ERROR("unsupported cipher");
  }

  std::unique_ptr<cryptor> make_decryptor(crypto_cipher cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    switch (cipher) {
      case crypto_cipher::aes_128_cbc:
      case crypto_cipher::aes_192_cbc:
      case crypto_cipher::aes_256_cbc:
        if (iv_size != 16) {
          throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
        }
        return std::unique_ptr<cryptor>(new aes_cryptor_impl(kCCDecrypt, key_data, key_size, iv_data, 0));
    }
    throw BRIGID_LOGIC_ERROR("unsupported cipher");
  }

  std::unique_ptr<hasher> make_hasher(crypto_hash hash) {
    switch (hash) {
      case crypto_hash::sha1:
        return std::unique_ptr<hasher>(new sha1_hasher_impl());
      case crypto_hash::sha256:
        return std::unique_ptr<hasher>(new sha256_hasher_impl());
      case crypto_hash::sha512:
        return std::unique_ptr<hasher>(new sha512_hasher_impl());
    }
    throw BRIGID_LOGIC_ERROR("unsupported hash");
  }
}
