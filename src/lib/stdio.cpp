// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/stdio.hpp>

#include <errno.h>
#include <stdio.h>
#include <string>
#include <system_error>

namespace brigid {
  file_handle_t make_file_handle(FILE* handle) {
    return file_handle_t(handle, &fclose);
  }

  file_handle_t open_file_handle(const std::string& path, const char* mode) {
    if (file_handle_t result = make_file_handle(fopen(path.c_str(), mode))) {
      return result;
    } else {
      int code = errno;
      throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
    }
  }
}
