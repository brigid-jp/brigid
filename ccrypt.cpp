#include "common.hpp"

#include <CommonCrypto/CommonCrypto.h>

namespace brigid {
  namespace {
    int impl_encrypt_string(lua_State* L) {
      return 0;
    }
  }

  void initialize_ccrypt(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, "ccrypt");
    lua_pushvalue(L, -2);
    lua_settable(L, -4);

    lua_pushstring(L, "encrypt_string");
    lua_pushcfunction(L, impl_encrypt_string);
    lua_settable(L, -3);

    lua_pop(L, 1);
  }
}
