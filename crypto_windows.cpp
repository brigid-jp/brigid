#include "common.hpp"

#include <windows.h>
#include <bcrypt.h>

#include <memory>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace {
    int impl_throw_error(NTSTATUS result) {
      std::ostringstream out;
      out << "error: " << result;
      throw std::runtime_error(out.str());
      return 0;
    }

    void impl_close_algorithm_provider(BCRYPT_ALG_HANDLE handle) {
      BCryptCloseAlgorithmProvider(handle, 0);
    }

    int impl_encrypt_string(lua_State* L) {
      const auto source = check_data(L, 1);
      const auto key = check_data(L, 2); // 256bit 32byte
      const auto iv = check_data(L, 3); // 128bit 16byte

      try {
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        NTSTATUS result = BCryptOpenAlgorithmProvider(
            &algorithm,
            BCRYPT_AES_ALGORITHM,
            nullptr,
            0);
        if (!BCRYPT_SUCCESS(result)) {
          return impl_throw_error(result);
        }
        std::unique_ptr<typename std::remove_pointer<BCRYPT_ALG_HANDLE>::type, decltype(&impl_close_algorithm_provider)> alg(algorithm, impl_close_algorithm_provider);

        DWORD object_length = 0;
        DWORD block_length = 0;
        DWORD result_data;

        result = BCryptGetProperty(
            alg.get(),
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&object_length),
            sizeof(object_length),
            &result_data,
            0);

        if (!BCRYPT_SUCCESS(result)) {
          return impl_throw_error(result);
        }

        result = BCryptGetProperty(
            alg.get(),
            BCRYPT_BLOCK_LENGTH,
            reinterpret_cast<PUCHAR>(&block_length),
            sizeof(block_length),
            &result_data,
            0);

        if (!BCRYPT_SUCCESS(result)) {
          return impl_throw_error(result);
        }

        BCRYPT_KEY_HANDLE key_handle = nullptr;
        std::vector<UCHAR> key_buffer(object_length);

        result = BCryptGenerateSymmetricKey(
            alg.get(),
            &key_handle,
            key_buffer.data(),
            key_buffer.size(),
            static_cast<PUCHAR>(const_cast<void*>(std::get<0>(key))),
            std::get<1>(key),
            0);

        if (!BCRYPT_SUCCESS(result)) {
          return impl_throw_error(result);
        }

        std::unique_ptr<typename std::remove_pointer<BCRYPT_KEY_HANDLE>::type, decltype(&BCryptDestroyKey)> k(key_handle, BCryptDestroyKey);

        std::vector<UCHAR> iv_buffer(16);
        memmove(iv_buffer.data(), std::get<0>(iv), 16);

        DWORD result_length = 0;
        result = BCryptEncrypt(
            k.get(),
            static_cast<PUCHAR>(const_cast<void*>(std::get<0>(source))),
            std::get<1>(source),
            nullptr,
            iv_buffer.data(),
            iv_buffer.size(),
            nullptr,
            0,
            &result_length,
            BCRYPT_BLOCK_PADDING);

        if (!BCRYPT_SUCCESS(result)) {
          return impl_throw_error(result);
        }

        std::vector<UCHAR> result_buffer(result_length);
        result = BCryptEncrypt(
            k.get(),
            static_cast<PUCHAR>(const_cast<void*>(std::get<0>(source))),
            std::get<1>(source),
            nullptr,
            iv_buffer.data(),
            iv_buffer.size(),
            result_buffer.data(),
            result_buffer.size(),
            &result_length,
            BCRYPT_BLOCK_PADDING);

        lua_pushlstring(L, reinterpret_cast<const char*>(result_buffer.data()), result_length);
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
