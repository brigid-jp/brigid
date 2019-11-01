#include "common.hpp"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace brigid {
  namespace {
    int impl_error(lua_State* L) {
      unsigned long e = ERR_get_error();
      char buffer[128] = { '\0' };
      ERR_error_string(e, buffer);
      return luaL_error(L, "%s", buffer);
    }

    void impl_throw_error() {
      unsigned long e = ERR_get_error();
      char buffer[128] = { '\0' };
      ERR_error_string(e, buffer);
      throw std::runtime_error(buffer);
    }

    struct string_reference {
      const char* data;
      size_t size;
    };

    inline string_reference checklstring(lua_State* L, int arg) {
      size_t size = 0;
      const char* data = luaL_checklstring(L, arg, &size);
      return { data, size };
    }

    int impl_encrypt_string(lua_State* L) {
      int top = lua_gettop(L);

      EVP_CIPHER_CTX* ctx = nullptr;
      try {
        const auto source = checklstring(L, 1);
        const auto key = checklstring(L, 2);
        const auto iv = checklstring(L, 3);

        if (ctx = EVP_CIPHER_CTX_new()) {
          // if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), 0, key_data, iv_data)) {
          // } else {
          // }
          EVP_CIPHER_CTX_free(ctx);
          return 0;
        } else {
          impl_throw_error();
        }
      } catch (const std::exception& e) {
        if (ctx) {
          EVP_CIPHER_CTX_free(ctx);
        }
        luaL_error(L, "%s", e.what());
      }
      return lua_gettop(L) - top;
    }
  }

  void initialize_evp(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, "evp");
    lua_pushvalue(L, -2);
    lua_settable(L, -4);

    lua_pushstring(L, "encrypt_string");
    lua_pushcfunction(L, impl_encrypt_string);
    lua_settable(L, -3);

    lua_pop(L, 1);
  }
}
