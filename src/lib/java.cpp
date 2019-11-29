// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "java.hpp"

extern "C" void* SDL_AndroidGetJNIEnv();

namespace brigid {
  JNIEnv* java_env() {
    return static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
  }
}
