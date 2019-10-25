#include "common.hpp"

namespace brigid {
  void initialize_evp(lua_State*);

  void initialize(lua_State* L) {
    initialize_evp(L);
  }
}

extern "C" int luaopen_brigid_core(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
