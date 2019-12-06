// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/version.hpp>
#include "test.hpp"

#include <stdint.h>
#include <string.h>
#include <iostream>

namespace {
  void test1() {
    const char* version = brigid::get_version();
    BRIGID_CHECK(version);
  }

  void test2() {
    const char* pb = brigid::get_version();
    const char* pe = pb + strlen(pb);

    int s = 1;
    for (const char* p = pb; p != pe; ++p) {
      char c = *p;
      switch (s) {
        case 1:
          if (0x30 <= c && c <= 0x39) {
            s = 2;
          } else {
            s = -1;
          }
          break;
        case 2:
          if (0x30 <= c && c <= 0x39) {
            s = 2;
          } else if (c == '.') {
            s = 3;
          } else {
            s = -1;
          }
          break;
        case 3:
          if (0x30 <= c && c <= 0x39) {
            s = 4;
          } else {
            s = -1;
          }
          break;
        case 4:
          if (0x30 <= c && c <= 0x39) {
            s = 4;
          } else {
            s = -1;
          }
          break;
      }
      BRIGID_CHECK(s != -1);
    }
    BRIGID_CHECK(s == 4);
  }

  BRIGID_MAKE_TEST_CASE(test1);
  BRIGID_MAKE_TEST_CASE(test2);
}
