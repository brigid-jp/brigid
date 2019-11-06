#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.hpp"

namespace brigid {
#ifdef HAVE_OPENSSL
  void initialize_evp(lua_State*);
#endif

  void initialize(lua_State* L) {
#ifdef HAVE_OPENSSL
    initialize_evp(L);
#endif
  }
}

extern "C" int luaopen_brigid_core(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
