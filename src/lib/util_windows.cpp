// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "util_windows.hpp"

#include <windows.h>

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

namespace brigid {
  namespace windows {
    namespace {
      inline bool make_utf8_string(const WCHAR* data, size_t size, std::string& output) {
        if (size == 0) {
          output.clear();
          return true;
        }

        int result = WideCharToMultiByte(
            CP_UTF8,
            0,
            data,
            static_cast<int>(size),
            nullptr,
            0,
            nullptr,
            nullptr);
        if (result == 0) {
          return false;
        }

        std::vector<char> buffer(result);

        result = WideCharToMultiByte(
            CP_UTF8,
            0,
            data,
            static_cast<int>(size),
            buffer.data(),
            static_cast<int>(buffer.size()),
            nullptr,
            nullptr);
        if (result == 0) {
          return false;
        }

        output.assign(buffer.data(), buffer.size());
        return true;
      }
    }

    bool make_windows_error_message(const char* name, uint32_t code, std::string& output) {
      HMODULE module = GetModuleHandle(name);
      if (!module) {
        return false;
      }

      WCHAR* buffer = nullptr;
      DWORD size = FormatMessageW(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
          module,
          code,
          0,
          reinterpret_cast<LPWSTR>(&buffer),
          0,
          nullptr);
      std::unique_ptr<WCHAR, decltype(&LocalFree)> data(buffer, &LocalFree);

      if (size < 2) {
        return false;
      }
      return make_utf8_string(data.get(), size - 2, output);
    }
  }
}
