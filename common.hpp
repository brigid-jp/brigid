#ifndef BRIGID_COMMON_HPP
#define BRIGID_COMMON_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <cstddef>
#include <tuple>

namespace brigid {
  std::tuple<const void*, std::size_t> check_data(lua_State*, int);
}

#endif
