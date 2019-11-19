// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/type_traits.hpp>

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

    class aes_encryptor_impl : public encryptor_impl {
    public:
      aes_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t)
        : cryptor_(make_cryptor_ref()) {
        CCCryptorRef cryptor = nullptr;
        check(CCCryptorCreateWithMode(kCCEncrypt, kCCModeCBC, kCCAlgorithmAES, kCCOptionPKCS7Padding, iv_data, key_data, key_size, nullptr, 0, 0, 0, &cryptor));
        cryptor_ = make_cryptor_ref(cryptor);
      }

      virtual size_t block_bytes() const {
        return 16;
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

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-256-cbc") {
      if (key_size != 32) {
        throw std::runtime_error("invalid key size");
      }
      if (iv_size != 16) {
        throw std::runtime_error("invalid iv size");
      }
      return std::unique_ptr<encryptor_impl>(new aes_encryptor_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
