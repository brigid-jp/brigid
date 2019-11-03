#include "common.hpp"

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace brigid {
  namespace {
    [[noreturn]] void impl_throw_error() {
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
      const auto source = checklstring(L, 1);
      const auto key = checklstring(L, 2); // 256bit 16byte
      const auto iv = checklstring(L, 3); // 128bit 8byte

      try {
        if (std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx { EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free }) {
          if (!EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, reinterpret_cast<const uint8_t*>(key.data), reinterpret_cast<const uint8_t*>(iv.data))) {
            impl_throw_error();
          }
          std::vector<char> buffer(source.size + 16);
          int size1 = buffer.size();
          if (!EVP_EncryptUpdate(ctx.get(), reinterpret_cast<uint8_t*>(buffer.data()), &size1, reinterpret_cast<const uint8_t*>(source.data), source.size)) {
            impl_throw_error();
          }
          int size2 = buffer.size() - size1;
          if (!EVP_EncryptFinal_ex(ctx.get(), reinterpret_cast<uint8_t*>(buffer.data()) + size1, &size2)) {
            impl_throw_error();
          }
          lua_pushlstring(L, buffer.data(), size1 + size2);
          return 1;
        } else {
          impl_throw_error();
        }
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
      }
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
