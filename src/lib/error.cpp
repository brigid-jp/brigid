// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>

#include <errno.h>
#include <sstream>
#include <string>
#include <system_error>

namespace brigid {
  std::string make_error_impl(const char* file, int line, const char* message) {
    std::ostringstream out;
    out << message << " at " << file << ":" << line;
    return out.str();
  }

  std::string make_error_impl(const char* file, int line, const errno_tag&) {
    int code = errno;
    return make_error_impl(file, line, std::generic_category().message(code).c_str(), make_error_code("error number", code));
  }
}
