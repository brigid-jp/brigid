// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>

#include <sstream>
#include <string>

namespace brigid {
  std::string make_error_impl(const char* file, int line, const char* message) {
    std::ostringstream out;
    out << message << " at " << file << ":" << line;
    return out.str();
  }
}
