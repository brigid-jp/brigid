// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_ERROR_WINDOWS_HPP
#define BRIGID_ERROR_WINDOWS_HPP

#include <stdint.h>
#include <string>

namespace brigid {
  namespace windows {
    std::string encode_utf8(const wchar_t*, size_t);
    std::wstring decode_utf8(const char*, size_t);
    std::wstring decode_utf8(const std::string&);
    bool get_error_message(const char*, uint32_t, std::string&);
  }
}

#endif
