#include "common.hpp"

#include <SDL.h>
#include <jni.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace {
    int impl_encrypt_string(lua_State* L) {
      const auto source = check_data(L, 1);
      const auto key = check_data(L, 2); // 256bit 32byte
      const auto iv = check_data(L, 3); // 128bit 16byte
      try {
        JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
        if (jclass c = env->FindClass("jp/brigid/core/Crypto")) {
          if (jmethodID m = env->GetStaticMethodID(c, "encryptString", "([B[B[B)[B")) {
            jbyteArray source_buffer = env->NewByteArray(std::get<1>(source));
            jbyte* source_ptr = env->GetByteArrayElements(source_buffer, nullptr);
            memcpy(source_ptr, std::get<0>(source), std::get<1>(source));
            env->ReleaseByteArrayElements(source_buffer, source_ptr, 0);

            jbyteArray key_buffer = env->NewByteArray(std::get<1>(key));
            jbyte* key_ptr = env->GetByteArrayElements(key_buffer, nullptr);
            memcpy(key_ptr, std::get<0>(key), std::get<1>(key));
            env->ReleaseByteArrayElements(key_buffer, key_ptr, 0);

            jbyteArray iv_buffer = env->NewByteArray(std::get<1>(iv));
            jbyte* iv_ptr = env->GetByteArrayElements(iv_buffer, nullptr);
            memcpy(iv_ptr, std::get<0>(iv), std::get<1>(iv));
            env->ReleaseByteArrayElements(iv_buffer, iv_ptr, 0);

            jobject result = env->CallStaticObjectMethod(c, m, source_buffer, key_buffer, iv_buffer);
            if (result) {
              if (jbyteArray r = reinterpret_cast<jbyteArray>(result)) {
                jsize n = env->GetArrayLength(r);
                jbyte* r_ptr = env->GetByteArrayElements(r, nullptr);
                lua_pushlstring(L, reinterpret_cast<const char*>(r_ptr), n);
                env->ReleaseByteArrayElements(r, r_ptr, 0);
                return 1;
              }
            }

            return 0;
          }
        }
        return 0;
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
