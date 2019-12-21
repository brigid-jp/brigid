// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/version.hpp>

namespace brigid {
  const char* get_version() {
#define m4_define(_, value) return #value;
#include "version.m4"
#undef m4_define
  }
}
