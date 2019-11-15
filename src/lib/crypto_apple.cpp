// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <CommonCrypto/CommonCrypto.h>

#include <stdexcept>
#include <sstream>
#include <type_traits>

namespace brigid {
  namespace {
    void check(CCCryptorStatus status) {
      if (status != kCCSuccess) {
        std::ostringstream out;
        out << "crypto_apple error number " << status;
        throw std::runtime_error(out.str());
      }
    }

    using cryptor_pointer_t = std::unique_ptr<typename std::remove_pointer<CCCryptorRef>::type, decltype(&CCCryptorRelease)>;

    class aes_encryptor_impl : public encryptor_impl {
    public:
      aes_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t)
        : cryptor_(create_cryptor(key_data, key_size, iv_data)) {}

      virtual size_t block_bytes() const {
        return 16;
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool do_final) {
        size_t size1 = 0;
        size_t size2 = 0;
        check(CCCryptorUpdate(cryptor_.get(), in_data, in_size, out_data, out_size, &size1));
        if (do_final) {
          check(CCCryptorFinal(cryptor_.get(), out_data + size1, out_size - size1, &size2));
        }
        return size1 + size2;
      }

    private:
      std::unique_ptr<typename std::remove_pointer<CCCryptorRef>::type, decltype(&CCCryptorRelease)> cryptor_;

      static cryptor_pointer_t create_cryptor(const char* key_data, size_t key_size, const char* iv_data) {
        CCCryptorRef cryptor = nullptr;
        check(CCCryptorCreate(kCCEncrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding, key_data, key_size, iv_data, &cryptor));
        return cryptor_pointer_t(cryptor, &CCCryptorRelease);
      }
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new aes_encryptor_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
