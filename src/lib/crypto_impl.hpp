// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_IMPL_HPP
#define BRIGID_CRYPTO_IMPL_HPP

#include <brigid/crypto.hpp>

#include <stddef.h>

namespace brigid {
  void check_cipher(crypto_cipher, size_t);
  void check_cipher(crypto_cipher, size_t, size_t);
}

#endif
