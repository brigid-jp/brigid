#include "common.hpp"

namespace brigid {
  std::tuple<const void*, std::size_t> check_data(lua_State* L, int arg) {
    // string
    size_t size = 0;
    const void* data = luaL_checklstring(L, arg, &size);
    return std::make_tuple(data, size);
  }
}
