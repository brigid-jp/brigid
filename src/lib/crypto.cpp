// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

namespace brigid {
  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string&, const char*, size_t, const char*, size_t);
  std::unique_ptr<decryptor_impl> make_decryptor_impl(const std::string&, const char*, size_t, const char*, size_t);

  namespace {
    void check(const std::string& cipher, size_t key_size, size_t iv_size) {
      if (cipher == "aes-128-cbc") {
        if (key_size != 16) {
          throw std::runtime_error("invalid key size");
        }
        if (iv_size != 16) {
          throw std::runtime_error("invalid iv size");
        }
      } else if (cipher == "aes-192-cbc") {
        if (key_size != 24) {
          throw std::runtime_error("invalid key size");
        }
        if (iv_size != 16) {
          throw std::runtime_error("invalid iv size");
        }
      } else if (cipher == "aes-256-cbc") {
        if (key_size != 32) {
          throw std::runtime_error("invalid key size");
        }
        if (iv_size != 16) {
          throw std::runtime_error("invalid iv size");
        }
      } else {
        throw std::runtime_error("unsupported cipher");
      }
    }
  }

  encryptor_impl::~encryptor_impl() {}

  encryptor::encryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check(cipher, key_size, iv_size);
    impl_ = make_encryptor_impl(cipher, key_data, key_size, iv_data, iv_size);
  }

  size_t encryptor::update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
    return impl_->update(in_data, in_size, out_data, out_size, padding);
  }

  decryptor_impl::~decryptor_impl() {}

  decryptor::decryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    check(cipher, key_size, iv_size);
    impl_ = make_decryptor_impl(cipher, key_data, key_size, iv_data, iv_size);
  }

  size_t decryptor::update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
    return impl_->update(in_data, in_size, out_data, out_size, padding);
  }
}
