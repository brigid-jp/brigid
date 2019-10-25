#include "common.hpp"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

namespace brigid {
  int impl_encrypt(lua_State* L) {
    return 0;
  }

  void initialize_evp(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, "evp");
    lua_pushvalue(L, -2);
    lua_settable(L, -4);

    lua_pushstring(L, "encrypt");
    lua_pushcfunction(L, impl_encrypt);
    lua_settable(L, -3);

    lua_pop(L, 1);
  }
}
