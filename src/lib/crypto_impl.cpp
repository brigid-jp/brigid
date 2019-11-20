// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "crypto_impl.hpp"

#include <stdexcept>

namespace brigid {
  void check_cipher(const std::string& cipher, size_t key_size, size_t iv_size) {
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
