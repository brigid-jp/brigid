#include "common.hpp"

extern "C" int luaopen_brigid_core(lua_State* L) {
  lua_newtable(L);
  return 1;
}
