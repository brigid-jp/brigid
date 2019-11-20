// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_IMPL_HPP
#define BRIGID_CRYPTO_IMPL_HPP

#include <stddef.h>
#include <string>

namespace brigid {
  void check_cipher(const std::string&, size_t, size_t);
}

#endif
