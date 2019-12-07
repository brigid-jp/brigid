// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "util_windows.hpp"

#define NOMINMAX
#include <windows.h>

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

namespace brigid {
  namespace windows {
    std::string encode_utf8(const wchar_t* data, size_t size) {
      if (size == 0) {
        return std::string();
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
        throw BRIGID_ERROR("cannot WideCharToMultiByte");
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
        throw BRIGID_ERROR("cannot WideCharToMultiByte");
      }

      return std::string(buffer.data(), buffer.size());
    }

    std::wstring decode_utf8(const char* data, size_t size) {
      if (size == 0) {
        return std::wstring();
      }

      int result = MultiByteToWideChar(
          CP_UTF8,
          0,
          data,
          static_cast<int>(size),
          nullptr,
          0);
      if (result == 0) {
        throw BRIGID_ERROR("cannot MultiByteToWideChar");
      }

      std::vector<WCHAR> buffer(result);

      result = MultiByteToWideChar(
          CP_UTF8,
          0,
          data,
          static_cast<int>(size),
          buffer.data(),
          static_cast<int>(buffer.size()));
      if (result == 0) {
        throw BRIGID_ERROR("cannot MultiByteToWideChar");
      }

      return std::wstring(buffer.data(), buffer.size());
    }

    std::wstring decode_utf8(const std::string& source) {
      return decode_utf8(source.data(), source.size());
    }

    bool get_error_message(const char* name, uint32_t code, std::string& output) {
      try {
        HMODULE module = GetModuleHandle(name);
        if (!module) {
          return false;
        }

        WCHAR* buffer = nullptr;
        DWORD result = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            module,
            code,
            0,
            reinterpret_cast<LPWSTR>(&buffer),
            0,
            nullptr);
        std::unique_ptr<WCHAR, decltype(&LocalFree)> data(buffer, &LocalFree);

        if (result < 2) {
          return false;
        }

        output = encode_utf8(data.get(), result - 2);
        return true;
      } catch (...) {
      }
      return false;
    }
  }
}
