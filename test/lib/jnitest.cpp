// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "JNITest.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <brigid/crypto.hpp>

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

static const std::string data { "The quick brown fox jumps over the lazy dog" };
static const std::string key { "01234567890123456789012345678901" };
static const std::string iv { "01234567890123456" };

JNIEXPORT void JNICALL Java_JNITest_test(JNIEnv* env, jclass) {
  access_jnienv(env);
  std::cout << "test...\n";
  std::cout << "env: " << env << "\n";
  try {
    std::vector<char> buffer(data.size() + 16);
    brigid::encryptor encryptor { "aes-256-cbc", key.data(), key.size(), iv.data(), iv.size() };
    // size_t result = encryptor.update(data.data(), data.size(), buffer.data(), buffer.size(), true);
  } catch (const std::exception& e) {
    std::cerr << "caught exception " << e.what() << "\n";
  }
  std::cout << "done";
}
