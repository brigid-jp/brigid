// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"

#include "JavaTest.h"

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
  return brigid::run_test_cases(0, nullptr);
}
