// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "JNITest.h"

#include <iomanip>
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
static const std::string iv { "0123456789012345" };

static const char* expect_data =
  "\xE0\x6F\x63\xA7\x11\xE8\xB7\xAA\x9F\x94\x40\x10\x7D\x46\x80\xA1"
  "\x17\x99\x43\x80\xEA\x31\xD2\xA2\x99\xB9\x53\x02\xD4\x39\xB9\x70"
  "\x2C\x8E\x65\xA9\x92\x36\xEC\x92\x07\x04\x91\x5C\xF1\xA9\x8A\x44";
static const size_t expect_size = 48;

JNIEXPORT void JNICALL Java_JNITest_test(JNIEnv* env, jclass) {
  access_jnienv(env);
  std::cout << "test...\n";
  std::cout << "env: " << env << "\n";
  try {
    std::vector<char> buffer(data.size() + 16);
    brigid::encryptor encryptor { "aes-256-cbc", key.data(), key.size(), iv.data(), iv.size() };
    size_t result = encryptor.update(data.data(), data.size(), buffer.data(), buffer.size(), true);
    // size_t result = 48;
    std::cout << result << "\n";
    buffer.resize(result);
    for (size_t i = 0; i < result; ++i) {
      std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(static_cast<uint8_t>(buffer[i]));
      if (i % 8 == 7) {
        std::cout << "\n";
      } else {
        std::cout << " ";
      }
    }

    std::cout << std::equal(buffer.begin(), buffer.end(), expect_data) << "\n";
  } catch (const std::exception& e) {
    std::cerr << "caught exception " << e.what() << "\n";
  }
  std::cout << "done\n";
}
