// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <windows.h>
#include <winhttp.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace {
    template <class T>
    using remove_pointer_t = typename std::remove_pointer<T>::type;

    std::wstring make_string(const std::string& source) {
      if (source.empty()) {
        return std::wstring();
      }
      int size = MultiByteToWideChar(
          CP_UTF8,
          0,
          source.data(),
          static_cast<int>(source.size()),
          nullptr,
          0);
      if (size == 0) {
        throw std::runtime_error("cannot WideCharToMultiByte");
      }
      std::vector<WCHAR> buffer(size);
      int result = MultiByteToWideChar(
          CP_UTF8,
          0,
          source.data(),
          static_cast<int>(source.size()),
          buffer.data(),
          static_cast<int>(buffer.size()));
      if (result == 0) {
        throw std::runtime_error("cannot WideCharToMultiByte");
      }
      return std::wstring(buffer.data(), buffer.size());
    }

    URL_COMPONENTS make_url_components(const std::string& source) {
      URL_COMPONENTS result = {};
      result.dwStructSize = sizeof(result);
      result.dwSchemeLength = -1;
      result.dwHostNameLength = -1;
      result.dwUrlPathLength = -1;
      result.dwExtraInfoLength = -1;
      std::wstring url = make_string(source);
      if (WinHttpCrackUrl(url.data(), static_cast<DWORD>(url.size()), 0, &result)) {
        return result;
      } else {
        throw std::runtime_error("cannot WinHttpCrackUrl");
      }
    }
  }

  void debug(int key, const std::string& message) {
    std::ostringstream out;
    out << key << " " << std::this_thread::get_id() << " " << message << "\n";
    std::cout << out.str();
  }

  void http(int key, const std::string& url) {
    debug(key, "start");

    std::wstring agent = make_string("brigid");

    HINTERNET session_handle = WinHttpOpen(
        agent.c_str(),
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!session_handle) {
      throw std::runtime_error("cannot WinHttpOpen");
    }
    std::unique_ptr<remove_pointer_t<HINTERNET>, decltype(&WinHttpCloseHandle)> session(session_handle, &WinHttpCloseHandle);

    URL_COMPONENTS url_c = make_url_components(url);

    std::wcout
        << url_c.lpszHostName << L"\n"
        << url_c.nPort << L"\n";

    HINTERNET connection_handle = WinHttpConnect(
        session.get(),
        url_c.lpszHostName,
        url_c.nPort,
        0);

    if (!connection_handle) {
      throw std::runtime_error("cannot WinHttpConnect");
    }
    std::unique_ptr<remove_pointer_t<HINTERNET>, decltype(&WinHttpCloseHandle)> connection(connection_handle, &WinHttpCloseHandle);

    debug(key, "done");
  }
}
