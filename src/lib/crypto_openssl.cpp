// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <openssl/err.h>
#include <openssl/evp.h>

#include <sstream>
#include <stdexcept>
#include <vector>

namespace brigid {
  namespace {
    template <typename T>
    T check(T result) {
      if (!result) {
        auto code = ERR_get_error();
        std::vector<char> buffer(256);
        ERR_error_string_n(code, buffer.data(), buffer.size());
        std::ostringstream out;
        out << "crypto_openssl error " << buffer.data();
        throw std::runtime_error(out.str());
      }
      return result;
    }

    using cipher_ctx_pointer_t = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

    class aes_256_cbc_encryptor_impl : public encryptor_impl {
    public:
      aes_256_cbc_encryptor_impl(const char* key_data, size_t, const char* iv_data, size_t)
        : ctx_(check(EVP_CIPHER_CTX_new()), EVP_CIPHER_CTX_free) {
        check(EVP_EncryptInit_ex(ctx_.get(), EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key_data), reinterpret_cast<const unsigned char*>(iv_data)));
      }

      virtual size_t block_bytes() const {
        return 16;
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
      cipher_ctx_pointer_t ctx_;
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new aes_256_cbc_encryptor_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
