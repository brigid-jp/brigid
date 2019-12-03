// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>
#include "error.hpp"
#include "http_impl.hpp"
#include "type_traits.hpp"
#include "util_windows.hpp"

#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace brigid {
  namespace {
    using namespace windows;

    template <class T>
    T check(T result) {
      if (!result) {
        DWORD code = GetLastError();
        std::string message;
        if (get_error_message("winhttp.dll", code, message)) {
          throw BRIGID_ERROR(message, make_error_code("winhttp error", code));
        } else {
          throw BRIGID_ERROR(make_error_code("winhttp error", code));
        }
      }
      return result;
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
          http_authentication_scheme auth_scheme,
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
          write_cb_(write_cb),
          auth_scheme_(auth_scheme),
          username_(decode_utf8(username)),
          password_(decode_utf8(password)) {}

      virtual void http_session_impl::request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data,
          size_t size) {
        std::wstring url_(decode_utf8(url));
        URL_COMPONENTS url_components = {};
        url_components.dwStructSize = sizeof(url_components);
        url_components.dwSchemeLength = -1;
        url_components.dwHostNameLength = -1;
        url_components.dwUrlPathLength = -1;
        url_components.dwExtraInfoLength = 0;
        check(WinHttpCrackUrl(url_.data(), static_cast<DWORD>(url_.size()), 0, &url_components));

        internet_handle_t connection = make_internet_handle(check(WinHttpConnect(
            session_.get(),
            std::wstring(url_components.lpszHostName, url_components.dwHostNameLength).c_str(),
            url_components.nPort,
            0)));

        internet_handle_t request = make_internet_handle(check(WinHttpOpenRequest(
            connection.get(),
            decode_utf8(method).c_str(),
            std::wstring(url_components.lpszUrlPath, url_components.dwUrlPathLength).c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            url_components.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)));

        for (const auto& field : header) {
          check(WinHttpAddRequestHeaders(
              request.get(),
              decode_utf8(field.first + ": " + field.second).c_str(),
              -1,
              WINHTTP_ADDREQ_FLAG_ADD));
        }

        DWORD code = send(request.get(), 0, body, data, size);

        if (header_cb_) {
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

          std::string header = encode_utf8(buffer.data(), buffer.size());
          http_header_parser parser;
          parser.parse(header.data(), header.size());
          if (!header_cb_(code, parser.get())) {
            throw BRIGID_ERROR("canceled");
          }
        }

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

          if (write_cb_) {
            if (!write_cb_(buffer.data(), buffer.size())) {
              throw BRIGID_ERROR("canceled");
            }
          }
        }
      }

    private:
      internet_handle_t session_;
      std::function<bool (size_t, size_t)> progress_cb_;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
      http_authentication_scheme auth_scheme_;
      std::wstring username_;
      std::wstring password_;

      DWORD send(
          HINTERNET request,
          DWORD auth_scheme,
          http_request_body body,
          const char* data,
          size_t size) {
        if (auth_scheme == 0) {
          switch (auth_scheme_) {
            case http_authentication_scheme::none:
              break;
            case http_authentication_scheme::basic:
              auth_scheme = WINHTTP_AUTH_SCHEME_BASIC;
              break;
            case http_authentication_scheme::digest:
              auth_scheme = WINHTTP_AUTH_SCHEME_DIGEST;
              break;
            case http_authentication_scheme::any:
              break;
          }
        }

        if (auth_scheme != 0) {
          check(WinHttpSetCredentials(
              request,
              WINHTTP_AUTH_TARGET_SERVER,
              auth_scheme,
              username_.c_str(),
              password_.c_str(),
              nullptr));
        }

        DWORD total = WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH;
        std::unique_ptr<http_reader> reader(make_http_reader(body, data, size));
        if (reader) {
          total = static_cast<DWORD>(reader->total());
        }

        check(WinHttpSendRequest(
            request,
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
                request,
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
            request,
            nullptr));

        DWORD code = 0;
        DWORD n = sizeof(code);
        check(WinHttpQueryHeaders(
            request,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &code,
            &n,
            WINHTTP_NO_HEADER_INDEX));

        if (code == 401 && auth_scheme_ == http_authentication_scheme::any && auth_scheme == 0) {
          DWORD schemes = 0;
          DWORD scheme = 0;
          DWORD target = 0;

          check(WinHttpQueryAuthSchemes(
              request,
              &schemes,
              &scheme,
              &target));
          if (scheme == WINHTTP_AUTH_SCHEME_BASIC || scheme == WINHTTP_AUTH_SCHEME_DIGEST) {
            auth_scheme = scheme;
          } else if (schemes & WINHTTP_AUTH_SCHEME_DIGEST) {
            auth_scheme = WINHTTP_AUTH_SCHEME_DIGEST;
          } else if (schemes & WINHTTP_AUTH_SCHEME_BASIC) {
            auth_scheme = WINHTTP_AUTH_SCHEME_BASIC;
          }

          if (auth_scheme != 0) {
            return send(request, auth_scheme, body, data, size);
          }
        }

        return code;
      }
    };
  }

  http_initializer::http_initializer() {}
  http_initializer::~http_initializer() {}

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (size_t, size_t)> progress_cb,
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      http_authentication_scheme auth_scheme,
      const std::string& username,
      const std::string& password) {
    return std::unique_ptr<http_session>(new http_session_impl(progress_cb, header_cb, write_cb, auth_scheme, username, password));
  }
}
