// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <stdio.h>
#include <vector>

namespace brigid {
  namespace {
    class file_writer_t : private noncopyable {
    public:
      explicit file_writer_t(const std::string& path) {
      }

    private:
    };
  }
}
