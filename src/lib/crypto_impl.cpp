// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "crypto_impl.hpp"
#include "error.hpp"

namespace brigid {
  void check_cipher(const std::string& cipher, size_t key_size, size_t iv_size) {
    size_t aes_key_size = 0;
    size_t aes_block_size = 16;

    if (cipher == "aes-128-cbc") {
      aes_key_size = 16;
    } else if (cipher == "aes-192-cbc") {
      aes_key_size = 24;
    } else if (cipher == "aes-256-cbc") {
      aes_key_size = 32;
    } else {
      throw BRIGID_ERROR("unsupported cipher");
    }

    if (key_size != aes_key_size) {
      throw BRIGID_ERROR("invalid key size");
    }
    if (iv_size != aes_block_size) {
      throw BRIGID_ERROR("invalid iv size");
    }
  }
}
