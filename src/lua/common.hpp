// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_COMMON_HPP
#define BRIGID_COMMON_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <stddef.h>
#include <string>

namespace brigid {
  class luax_data_reference {
  public:
    luax_data_reference(const char*, size_t);

    const char* data() const;

    size_t size() const;

    std::string to_str() const;

  private:
    const char* data_;
    size_t size_;
  };

  using luax_function_t = void (*)(lua_State*);

  int luax_abs_index(lua_State*);

  luax_data_reference luax_check_data(lua_State*, int);

  void luax_push(lua_State*, const char*, size_t);

  void luax_push(lua_State*, luax_function_t);

  void luax_set_field(lua_State*, int, const char*, luax_function_t);
}

#endif
