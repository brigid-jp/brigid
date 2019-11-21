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

    template <class T>
    T check(T result) {
      if (!result) {
        std::ostringstream out;
        out << "http_windows error number " << GetLastError();
        throw std::runtime_error(out.str());
      }
      return result;
    }

    using internet_handle_t = std::unique_ptr<remove_pointer_t<HINTERNET>, decltype(&WinHttpCloseHandle)>;

    internet_handle_t make_internet_handle(HINTERNET handle = nullptr) {
      return internet_handle_t(handle, &WinHttpCloseHandle);
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

    internet_handle_t session = make_internet_handle(check(WinHttpOpen(
        agent.c_str(),
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0)));

    URL_COMPONENTS url_c = make_url_components(url);
    std::wcout << std::wstring(url_c.lpszHostName, url_c.dwHostNameLength) << L"\n";
    std::wcout << std::wstring(url_c.lpszUrlPath, url_c.dwUrlPathLength) << L"\n";

    debug(key, "connect");

    internet_handle_t connection = make_internet_handle(check(WinHttpConnect(
        session.get(),
        std::wstring(url_c.lpszHostName, url_c.dwHostNameLength).c_str(),
        url_c.nPort,
        0)));

    debug(key, "request");

    internet_handle_t request = make_internet_handle(check(WinHttpOpenRequest(
        connection.get(),
        make_string("GET").c_str(),
        std::wstring(url_c.lpszUrlPath, url_c.dwUrlPathLength).c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        url_c.nScheme == INTERNET_SCHEME_HTTPS
          ? WINHTTP_FLAG_SECURE
          : 0)));

    debug(key, "send");

    check(WinHttpSendRequest(
        request.get(),
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH,
        0)); // nullptr cause error. why?

    debug(key, "recv");

    check(WinHttpReceiveResponse(
        request.get(),
        nullptr));

    debug(key, "read");

    while (true) {
      DWORD available = 0;
      check(WinHttpQueryDataAvailable(request.get(), &available));
      if (!available) {
        break;
      }
      std::vector<char> buffer(available);
      DWORD size = 0;
      check(WinHttpReadData(
          request.get(),
          buffer.data(),
          static_cast<DWORD>(buffer.size()),
          &size));

      buffer.resize(size);
      std::cout << std::string(buffer.data(), buffer.size()) << "\n";
    }

    debug(key, "done");
  }
}
