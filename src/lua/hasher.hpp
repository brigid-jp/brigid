// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HASHER_HPP
#define BRIGID_HASHER_HPP

#include <brigid/crypto.hpp>

#include <lua.hpp>

namespace brigid {
  crypto_hash check_hash(lua_State*, int);
}

#endif
