#include "common.hpp"

#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

namespace brigid {
  namespace {
    int impl_encrypt_string(lua_State* L) {
      BCRYPT_ALG_HANDLE algorithm = nullptr;
      NTSTATUS result = -1;

      result = BCryptOpenAlgorithmProvider(
          &algorithm,
          BCRYPT_AES_ALGORITHM,
          nullptr,
          0);

      return 0;
    }
  }

  void initialize_crypto(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, "crypto");
    lua_pushvalue(L, -2);
    lua_settable(L, -4);

    lua_pushstring(L, "encrypt_string");
    lua_pushcfunction(L, impl_encrypt_string);
    lua_settable(L, -3);

    lua_pop(L, 1);
  }
}
