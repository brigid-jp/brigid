// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>
#include "error.hpp"
#include "http_impl.hpp"
#include "type_traits.hpp"

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <string>
#include <vector>

namespace brigid {
  http_initializer::http_initializer() {}
  http_initializer::~http_initializer() {}

  namespace {
    template <class T>
    T check(T result) {
      if (!result) {
        throw BRIGID_ERROR(GetLastError());
      }
      return result;
    }

    using native_string_t = std::basic_string<WCHAR>;

    native_string_t to_native_string(const std::string& source) {
      if (source.empty()) {
        return native_string_t();
      }
      int result = check(MultiByteToWideChar(
          CP_UTF8,
          0,
          source.data(),
          static_cast<int>(source.size()),
          nullptr,
          0));
      std::vector<WCHAR> buffer(result);
      check(MultiByteToWideChar(
          CP_UTF8,
          0,
          source.data(),
          static_cast<int>(source.size()),
          buffer.data(),
          static_cast<int>(buffer.size())));
      return native_string_t(buffer.begin(), buffer.end());
    }

    native_string_t to_native_string(const WCHAR* data, size_t size) {
      return native_string_t(data, size);
    }

    std::string to_string(const WCHAR* data, size_t size) {
      if (size == 0) {
        return std::string();
      }
      int result = check(WideCharToMultiByte(
          CP_UTF8,
          0,
          source.data(),
          static_cast<int>(source.size()),
          nullptr,
          0,
          nullptr,
          nullptr));
      std::vector<char> buffer(result);
      check(WideCharToMultiByte(
          CP_UTF8,
          0,
          source.data(),
          static_cast<int>(source.size()),
          buffer.data(),
          static_cast<int>(buffer.size()),
          nullptr,
          nullptr));
      return std::string(buffer.data(), buffer.size());
    }

    using internet_handle_t = std::unique_ptr<remove_pointer_t<HINTERNET>, decltype(&WinHttpCloseHandle)>;

    internet_handle_t make_internet_handle(HINTERNET handle = nullptr) {
      return internet_handle_t(handle, &WinHttpCloseHandle);
    }

    class http_session_impl : public http_session, private noncopyable {
    public:
      http_session_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : session_(make_internet_handle(check(WinHttpOpen(
              nullptr,
              WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
              WINHTTP_NO_PROXY_NAME,
              WINHTTP_NO_PROXY_BYPASS,
              0)))),
          progress_cb_(progress_cb),
          header_cb_(header_cb),
          write_cb_(write_cb) {}

      virtual void http_session_impl::request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data,
          size_t size) {
        native_string_t native_method = to_native_string(method);
        native_string_t native_url = to_native_string(url);

        URL_COMPONENTS url_components = {};
        url_components.dwStructSize = sizeof(url_components);
        url_components.dwSchemeLength = -1;
        url_components.dwHostNameLength = -1;
        url_components.dwUrlPathLength = -1;
        url_components.dwExtraInfoLength = -1;
        check(WinHttpCrackUrl(native_url.data(), static_cast<DWORD>(native_url.size()), 0, &url_components));

        native_string_t native_host_name = to_native_string(url_components.lpszHostName, url_components.dwHostNameLength);
        native_string_t native_url_path = to_native_string(url_components.lpszUrlPath, url_components.dwUrlPathLength);

        internet_handle_t connection = make_internet_handle(check(WinHttpConnect(
            session_.get(),
            native_host_name.c_str(),
            url_components.nPort,
            0)));

        internet_handle_t request = make_internet_handle(check(WinHttpOpenRequest(
            connection.get(),
            native_method.c_str(),
            native_url_path.c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            url_components.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)));

        for (const auto& field : header) {
          native_string_t native_field = to_native_string(field.first + ": " + field.second);
          check(WinHttpAddRequestHeaders(
              request.get(),
              native_field.c_str(),
              static_cast<DWORD>(native_field.size()),
              WINHTTP_ADDREQ_FLAG_ADD));
        }

        DWORD total = WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH;
        std::unique_ptr<http_reader> reader(make_http_reader(body, data, size));
        if (reader) {
          total = static_cast<DWORD>(reader->total());
        }

        check(WinHttpSendRequest(
            request.get(),
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            total,
            0));

        if (reader) {
          std::vector<char> buffer(4096);
          while (true) {
            size_t result = reader->read(buffer.data(), buffer.size());
            if (result == 0) {
              break;
            }
            check(WinHttpWriteData(
                request.get(),
                buffer.data(),
                static_cast<DWORD>(result),
                nullptr));
            if (progress_cb_) {
              if (!progress_cb_(reader->now(), reader->total())) {
                throw BRIGID_ERROR("canceled");
              }
            }
          }
        }

        check(WinHttpReceiveResponse(
            request.get(),
            nullptr));

        {
          DWORD size = 0;
          BOOL result = WinHttpQueryHeaders(
              request.get(),
              WINHTTP_QUERY_RAW_HEADERS_CRLF,
              WINHTTP_HEADER_NAME_BY_INDEX,
              WINHTTP_NO_OUTPUT_BUFFER,
              &size,
              WINHTTP_NO_HEADER_INDEX);
          check(!result && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

          std::vector<WCHAR> buffer(size);

          check(WinHttpQueryHeaders(
              request.get(),
              WINHTTP_QUERY_RAW_HEADERS_CRLF,
              WINHTTP_HEADER_NAME_BY_INDEX,
              buffer.data(),
              &size,
              WINHTTP_NO_HEADER_INDEX));

          buffer.resize(size);

          std::string header = to_string(buffer.data(), buffer.size());
        }
      }

    private:
      internet_handle_t session_;
      std::function<bool (size_t, size_t)> progress_cb_;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
    };
  }

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (size_t, size_t)> progress_cb,
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      bool credential,
      const std::string& username,
      const std::string& password) {
    return std::unique_ptr<http_session>(new http_session_impl(progress_cb, header_cb, write_cb, credential, username, password));
  }
}
