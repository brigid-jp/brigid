// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_UTIL_JAVA_HPP
#define BRIGID_UTIL_JAVA_HPP

#include <lua.hpp>

namespace brigid {
  namespace lua {
    using cxx_function_t = void (*)(lua_State*);

    int abs_index(lua_State*, int);
    void push(lua_State*, cxx_function_t);
  }
}

#endif
