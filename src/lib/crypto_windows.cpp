// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <stdexcept>

namespace brigid {
  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    throw std::runtime_error("unsupported cipher");
  }
}
