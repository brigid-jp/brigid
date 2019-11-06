#include "common.hpp"

#include <CommonCrypto/CommonCrypto.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace {
    [[noreturn]] void impl_throw_error(CCCryptorStatus result) {
      std::ostringstream out;
      out << "error: " << result;
      throw std::runtime_error(out.str());
    }

    int impl_encrypt_string(lua_State* L) {
      const auto source = check_data(L, 1);
      const auto key = check_data(L, 2); // 256bit 16byte
      const auto iv = check_data(L, 3); // 128bit 8byte

      try {
        CCCryptorRef cryptor_ref = nullptr;
        auto result = CCCryptorCreate(
            kCCEncrypt,
            kCCAlgorithmAES,
            kCCOptionPKCS7Padding,
            std::get<0>(key),
            std::get<1>(key),
            std::get<0>(iv),
            &cryptor_ref);
        if (result != kCCSuccess) {
          impl_throw_error(result);
        }

        std::unique_ptr<typename std::remove_pointer<CCCryptorRef>::type, decltype(&CCCryptorRelease)> cryptor { cryptor_ref, CCCryptorRelease };

        std::vector<char> buffer(std::get<1>(source) + 16);
        size_t size1 = 0;
        size_t size2 = 0;

        result = CCCryptorUpdate(
            cryptor.get(),
            std::get<0>(source),
            std::get<1>(source),
            buffer.data(),
            buffer.size(),
            &size1);
        if (result != kCCSuccess) {
          impl_throw_error(result);
        }

        result = CCCryptorFinal(
            cryptor.get(),
            buffer.data() + size1,
            buffer.size() - size1,
            &size2);
        if (result != kCCSuccess) {
          impl_throw_error(result);
        }
        lua_pushlstring(L, buffer.data(), size1 + size2);
        return 1;
      } catch (const std::exception& e) {
        return luaL_error(L, "%s", e.what());
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
