// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_ERROR_WINDOWS_HPP
#define BRIGID_ERROR_WINDOWS_HPP

#include <stdint.h>
#include <string>

namespace brigid {
  namespace windows {
    bool make_windows_error_message(const char*, uint32_t, std::string&);
  }
}

#endif
