#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.hpp"

namespace brigid {
#if defined(HAVE_OPENSSL)
  void initialize_evp(lua_State*);
#elif defined(HAVE_COMMONCRYPTO_COMMONCRYPTO_H)
  void initialize_ccrypt(lua_State*);
#endif

  void initialize(lua_State* L) {
#if defined(HAVE_OPENSSL)
    initialize_evp(L);
#elif defined(HAVE_COMMONCRYPTO_COMMONCRYPTO_H)
    initialize_ccrypt(L);
#endif
  }
}

extern "C" int luaopen_brigid_core(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
