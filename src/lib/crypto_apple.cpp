// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "crypto_impl.hpp"
#include "type_traits.hpp"
#include <brigid/crypto.hpp>

#include <CommonCrypto/CommonCrypto.h>

#include <sstream>
#include <stdexcept>

namespace brigid {
  namespace {
    void check(CCCryptorStatus status) {
      if (status != kCCSuccess) {
        std::ostringstream out;
        out << "crypto_apple error number " << status;
        throw std::runtime_error(out.str());
      }
    }

    using cryptor_ref_t = std::unique_ptr<remove_pointer_t<CCCryptorRef>, decltype(&CCCryptorRelease)>;

    cryptor_ref_t make_cryptor_ref(CCCryptorRef cryptor = nullptr) {
      return cryptor_ref_t(cryptor, &CCCryptorRelease);
    }

    class aes_cryptor_impl : public cryptor {
    public:
      aes_cryptor_impl(CCOperation operation, const char* key_data, size_t key_size, const char* iv_data, size_t)
        : cryptor_(make_cryptor_ref()) {
        CCCryptorRef cryptor = nullptr;
        check(CCCryptorCreateWithMode(operation, kCCModeCBC, kCCAlgorithmAES, kCCOptionPKCS7Padding, iv_data, key_data, key_size, nullptr, 0, 0, 0, &cryptor));
        cryptor_ = make_cryptor_ref(cryptor);
      }

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
    };

    class aes_decryptor_impl : public cryptor {
    public:
      aes_decryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t)
        : cryptor_(make_cryptor_ref()) {
        CCCryptorRef cryptor = nullptr;
        check(CCCryptorCreateWithMode(kCCDecrypt, kCCModeCBC, kCCAlgorithmAES, kCCOptionPKCS7Padding, iv_data, key_data, key_size, nullptr, 0, 0, 0, &cryptor));
        cryptor_ = make_cryptor_ref(cryptor);
      }

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
    };
  }

  std::unique_ptr<cryptor> make_encryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check_cipher(cipher, key_size, iv_size);
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      return std::unique_ptr<cryptor>(new aes_cryptor_impl(kCCEncrypt, key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }

  std::unique_ptr<cryptor> make_decryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check_cipher(cipher, key_size, iv_size);
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      return std::unique_ptr<cryptor>(new aes_cryptor_impl(kCCDecrypt, key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
