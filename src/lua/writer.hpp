// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_WRITER_HPP
#define BRIGID_WRITER_HPP

#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include "write_urlencoded.hxx"

namespace brigid {
  template <class T, T* (*T_check_writer)(lua_State*, int, int)>
  struct writer {
    // https://url.spec.whatwg.org/#urlencoded-serializing
    static void write_urlencoded(lua_State* L) {
      T* self = T_check_writer(L, 1, check_validate_all);
      data_t data = check_data(L, 2);
      impl_write_urlencoded(self, data);
    }

    static void initialize(lua_State* L) {
      decltype(function<write_urlencoded>())::set_field(L, -1, "write_urlencoded");
    }
  };
}

#endif
