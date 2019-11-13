#include "common.hpp"
#include "module.hpp"

namespace brigid {
  void initialize_crypto(lua_State*);

  void initialize(lua_State* L) {
    initialize_crypto(L);
  }
}

extern "C" int luaopen_brigid_core(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
