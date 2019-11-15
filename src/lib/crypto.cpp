// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

namespace brigid {
  encryptor_impl::~encryptor_impl() {}

  encryptor::encryptor(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
    : impl_(make_encryptor_impl(cipher, key_data, key_size, iv_data, iv_size)) {}

  size_t encryptor::update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
    return impl_->update(in_data, in_size, out_data, out_size, padding);
  }
}
