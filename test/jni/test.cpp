// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "Test.h"

#include <iostream>

JNIEXPORT void JNICALL Java_Test_test(JNIEnv* env, jclass) {
  std::cout << "test...\n";
}
