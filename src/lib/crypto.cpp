// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

namespace brigid {
  encryptor_impl::~encryptor_impl() {}

  encryptor::encryptor(
      const std::string& cipher,
      const char* key_data,
      std::size_t key_size,
      const char* iv_data,
      std::size_t iv_size)
    : impl_(make_encryptor_impl(cipher)) {

    impl_->open(key_data, key_size, iv_data, iv_size);
  }
}
