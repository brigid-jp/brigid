// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "scope_exit.hpp"

#include <functional>

namespace brigid {
  scope_exit::scope_exit(std::function<void ()> function)
    : function_(function) {}

  scope_exit::~scope_exit() {
    try {
      function_();
    } catch (...) {}
  }
}
