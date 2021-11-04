// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "error.hpp"
#include "stdio.hpp"

#include <stdio.h>

namespace brigid {
  file_handle_t make_file_handle(FILE* handle) {
    return file_handle_t(handle, &fclose);
  }

  file_handle_t open_file_handle(const char* path, const char* mode) {
    if (file_handle_t result = make_file_handle(fopen(path, mode))) {
      return result;
    } else {
      throw BRIGID_SYSTEM_ERROR();
    }
  }
}
