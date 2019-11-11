#include "common.hpp"

#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")
// #pragma comment(lib, "crypt32.lib")

namespace brigid {
  namespace {
    int impl_encrypt_string(lua_State* L) {
      HCRYPTPROV prov = 0;
      if (!CryptAcquireContext(&prov, nullptr, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
      }

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
