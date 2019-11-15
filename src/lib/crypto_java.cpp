// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <jni.h>

extern "C" void* SDL_AndroidGetJNIEnv();

#include <iostream>

namespace brigid {
  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    std::cout << "env: " << env << "\n";
    throw std::runtime_error("unsupported cipher");
  }
}
