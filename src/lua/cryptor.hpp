// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTOR_HPP
#define BRIGID_CRYPTOR_HPP

#include <brigid/crypto.hpp>

#include <lua.hpp>

#include <memory>

namespace brigid {
  crypto_cipher check_cipher(lua_State*, int);
  void new_decryptor(lua_State*, std::unique_ptr<cryptor>&&, reference&&);
}

#endif
