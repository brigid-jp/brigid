// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_STDIO_HPP
#define BRIGID_STDIO_HPP

#include <stdio.h>
#include <memory>
#include <string>

namespace brigid {
  using file_handle_t = std::unique_ptr<FILE, decltype(&fclose)>;

  file_handle_t make_file_handle(FILE* = nullptr);
  file_handle_t open_file_handle(const char*, const char*);
}

#endif
