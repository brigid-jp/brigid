// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "JavaTest.h"

#include <lua.hpp>

#include <iostream>
#include <memory>
#include <vector>

extern "C" int luaopen_brigid(lua_State*);

void* access_jnienv(void* set_env = 0) {
  thread_local void* env = 0;
  if (set_env) {
    env = set_env;
  }
  return env;
}

extern "C" void* SDL_AndroidGetJNIEnv() {
  return access_jnienv();
}

JNIEXPORT jint JNICALL Java_JavaTest_test(JNIEnv* env, jclass, jbyteArray filename) {
  access_jnienv(env);

  jsize size = env->GetArrayLength(filename);
  std::vector<char> buffer(size + 1);
  env->GetByteArrayRegion(filename, 0, size, reinterpret_cast<jbyte*>(buffer.data()));
  if (env->ExceptionCheck()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return 1;
  }

  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  lua_State* L = state.get();

  luaL_openlibs(L);

  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_pushcfunction(L, luaopen_brigid);
  lua_setfield(L, -2, "brigid");
  lua_pop(L, 2);

  luaL_loadfile(L, buffer.data());
  if (lua_pcall(L, 0, 0, 0) != 0) {
    std::cerr << lua_tostring(L, -1) << "\n";
    return 1;
  }

  // luaopen_brigid(L);

  // open_brigid();
  // return brigid::run_test_cases(0, nullptr);
  return 0;
}
