// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "JavaTest.h"

#include <lua.hpp>

#include <memory>

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

JNIEXPORT jint JNICALL Java_JavaTest_test(JNIEnv* env, jclass) {
  access_jnienv(env);

  std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(), &lua_close);
  lua_State* L = state.get();

  luaopen_brigid(L);

  // open_brigid();
  // return brigid::run_test_cases(0, nullptr);
  return 0;
}
