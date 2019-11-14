// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <stdexcept>

namespace brigid {
  namespace {
    class aes256_encryptor_impl : public encryptor_impl {
    public:
      virtual void open(
          const char* key_data,
          std::size_t key_size,
          const char* iv_data,
          std::size_t iv_size) {
      }

      virtual std::size_t key_size() const {
        return 256;
      }

      virtual std::size_t block_size() const {
        return 128;
      }

    private:
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher) {
    if (cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new aes256_encryptor_impl());
    } else {
      throw std::runtime_error("unsupported");
    }
  }
}
