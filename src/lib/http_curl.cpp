// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include "error.hpp"
#include "http_curl.hpp"

#include <curl/curl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

namespace brigid {
  namespace {
    int http_initializer_counter = 0;
  }

  http_initializer::http_initializer() {
    if (++http_initializer_counter == 1) {
      curl_global_init(CURL_GLOBAL_ALL);
    }
  }

  http_initializer::~http_initializer() {
    if (--http_initializer_counter == 0) {
      curl_global_cleanup();
    }
  }

  namespace {
    // https://tools.ietf.org/html/rfc7230#section-3.2
    bool is_tchar(uint8_t c) {
      switch (c) {
        case '!':
        case '#':
        case '$':
        case '%':
        case '&':
        case '\'':
        case '*':
        case '+':
        case '-':
        case '.':
        case '^':
        case '_':
        case '`':
        case '|':
        case '~':
          return true;
      }
      return (0x30 <= c && c <= 0x39)
          || (0x41 <= c && c <= 0x5A)
          || (0x61 <= c && c <= 0x7A);
    }

    bool is_vchar(uint8_t c) {
      return (0x21 <= c && c <= 0x7E)
          || 0x80 <= c;
    }
  }

  http_header_parser::http_header_parser()
    : state_(1) {}

  bool http_header_parser::parse(const char* pb, size_t n) {
    if (state_ == 0) {
      state_ = 1;
      buffer_.clear();
      header_.clear();
    }

    if (state_ == 1) {
      state_ = 2;
      return false;
    }

    const char* pe = pb + n;
    const char* p1 = nullptr;
    const char* p2 = nullptr;
    int s = 1;
    const char* p = pe;
    while (p != pb) {
      --p;
      char c = *p;
      switch (s) {
        case 1:
          if (c == '\n') {
            s = 2;
          } else {
            s = 0;
          }
          break;
        case 2:
          if (c == '\r') {
            p1 = p;
            p2 = p;
            s = 3;
          } else {
            s = 0;
          }
          break;
        case 3:
          if (c == ' ' || c == '\t') {
            p2 = p;
          } else {
            s = 0;
          }
          break;
      }
      if (s == 0) {
        break;
      }
    }

    if (!p1 || !p2) {
      throw BRIGID_ERROR("cannot parse header");
    }

    if (p1 == pb) {
      if (state_ == 3) {
        parse_impl();
      }
      state_ = 0;
      return true;
    }

    const char* p3 = nullptr;
    s = 1;
    for (const char* p = pb; p != pe; ++p) {
      char c = *p;
      switch (s) {
        case 1:
          if (c == ' ' || c == '\t') {
            p3 = p;
          } else {
            s = 0;
          }
          break;
      }
      if (s == 0) {
        break;
      }
    }

    if (p3) {
      if (p3 < p2) {
        buffer_.append(" ");
        buffer_.append(p3 + 1, p2);
      }
      return false;
    }

    if (state_ == 2) {
      state_ = 3;
    } else {
      parse_impl();
    }
    buffer_.assign(pb, p2);
    return false;
  }

  const std::map<std::string, std::string>& http_header_parser::get() const {
    return header_;
  }

  void http_header_parser::parse_impl() {
    const char* pb = buffer_.data();
    const char* pe = pb + buffer_.size();
    int s = 1;

    const char* p1 = nullptr;
    const char* p2 = nullptr;
    const char* p3 = nullptr;
    for (const char* p = pb; p != pe; ++p) {
      char c = *p;
      switch (s) {
        case 1:
          if (is_tchar(c)) {
            s = 2;
          } else {
            s = 0;
          }
          break;
        case 2:
          if (c == ':') {
            p1 = p;
            s = 3;
          } else if (!is_tchar(c)) {
            s = 0;
          }
          break;
        case 3:
          if (is_vchar(c)) {
            p2 = p;
            s = 4;
          } else if (c != ' ' && c != '\t') {
            s = 0;
          }
          break;
        case 4:
          if (!is_vchar(c) && c != ' ' && c != '\t') {
            p3 = p;
            s = 0;
          }
      }
      if (s == 0) {
        break;
      }
    }

    if (!p1 || p3) {
      throw BRIGID_ERROR("cannot parse header");
    }

    if (p2) {
      header_[std::string(pb, p1)] = std::string(p2, pe);
    } else {
      header_[std::string(pb, p1)] = std::string();
    }
  }

  namespace {
    void check(CURLcode code) {
      if (code != CURLE_OK) {
        throw BRIGID_ERROR(curl_easy_strerror(code), code);
      }
    }

    CURL* check(CURL* handle) {
      if (!handle) {
        check(CURLE_FAILED_INIT);
      }
      return handle;
    }

    FILE* check(FILE* handle) {
      if (!handle) {
        throw BRIGID_ERROR("cannot fopen", errno);
      }
      return handle;
    }

    using easy_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

    easy_t make_easy(CURL* handle) {
      return easy_t(handle, &curl_easy_cleanup);
    }

    using file_t = std::unique_ptr<FILE, decltype(&fclose)>;

    file_t make_file(FILE* handle) {
      return file_t(handle, &fclose);
    }

    class http_session_impl : public http_session {
    public:
      http_session_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : handle(make_easy(check(curl_easy_init()))),
          progress_cb(progress_cb),
          header_cb(header_cb),
          write_cb(write_cb),
          credential(credential),
          username(username),
          password(password) {}

      virtual void request(const std::string&, const std::string&, const std::map<std::string, std::string>&, http_request_body, const char*, size_t);

      easy_t handle;
      std::function<bool (size_t, size_t)> progress_cb;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb;
      std::function<bool (const char*, size_t)> write_cb;
      bool credential;
      std::string username;
      std::string password;
    };

    class string_list {
    public:
      string_list()
        : slist_() {}

      ~string_list() {
        curl_slist_free_all(slist_);
      }

      void append(const std::string& string) {
        if (curl_slist* slist = curl_slist_append(slist_, string.c_str())) {
          slist_ = slist;
        } else {
          throw BRIGID_ERROR("cannot curl_slist_append");
        }
      }

      curl_slist* get() const {
        return slist_;
      }

    private:
      curl_slist* slist_;
    };

    class http_reader {
    public:
      http_reader() : now(), total() {}
      virtual ~http_reader() {}
      virtual size_t read(char*, size_t) = 0;

      size_t now;
      size_t total;
    };

    class http_data_reader : public http_reader {
    public:
      http_data_reader(const char* data, size_t size)
        : data_(data),
          size_(size) {
        total = size;
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = std::min(size, size_);
        memmove(data, data_, result);
        data_ += result;
        size_ -= result;
        now += result;
        return result;
      }

    private:
      const char* data_;
      size_t size_;
    };

    class http_file_reader : public http_reader {
    public:
      http_file_reader(const char* path)
        : handle_(make_file(check(fopen(path, "rb")))) {
        fseek(handle_.get(), 0, SEEK_END);
        total = ftell(handle_.get());
        fseek(handle_.get(), 0, SEEK_SET);
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = fread(data, 1, size, handle_.get());
        now += result;
        return result;
      }

    private:
      file_t handle_;
    };

    std::unique_ptr<http_reader> make_reader(http_request_body body, const char* data, size_t size) {
      switch (body) {
        case http_request_body::data:
          if (data) {
            return std::unique_ptr<http_reader>(new http_data_reader(data, size));
          }
          break;
        case http_request_body::file:
          return std::unique_ptr<http_reader>(new http_file_reader(data));
      }
      return std::unique_ptr<http_reader>();
    }

    class http_task {
    public:
      http_task(
          http_session_impl* session,
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data,
          size_t size)
        : session_(session),
          method_(method),
          url_(url),
          reader_(make_reader(body, data, size)) {
        for (const auto& field : header) {
          header_.append(field.first + ": " + field.second);
        }
      }

      void request() {
        CURL* handle = session_->handle.get();

        check(curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1));
        check(curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, method_.c_str()));
        if (method_ == "HEAD") {
          check(curl_easy_setopt(handle, CURLOPT_NOBODY, 1));
        }
        check(curl_easy_setopt(handle, CURLOPT_URL, url_.c_str()));
        check(curl_easy_setopt(handle, CURLOPT_HTTPHEADER, header_.get()));

        if (reader_) {
          check(curl_easy_setopt(handle, CURLOPT_UPLOAD, 1));
          check(curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, reader_->total));
          check(curl_easy_setopt(handle, CURLOPT_READFUNCTION, &http_task::read_cb));
          check(curl_easy_setopt(handle, CURLOPT_READDATA, this));
        }

        if (session_->header_cb) {
          check(curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, &http_task::header_cb));
          check(curl_easy_setopt(handle, CURLOPT_HEADERDATA, this));
        }

        if (session_->write_cb) {
          check(curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &http_task::write_cb));
          check(curl_easy_setopt(handle, CURLOPT_WRITEDATA, this));
        }

        if (session_->credential) {
          check(curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY));
          check(curl_easy_setopt(handle, CURLOPT_USERNAME, session_->username.c_str()));
          check(curl_easy_setopt(handle, CURLOPT_PASSWORD, session_->password.c_str()));
        }

        CURLcode code = curl_easy_perform(handle);
        if (exception_) {
          std::rethrow_exception(exception_);
        }
        check(code);
      }

      ~http_task() {
        curl_easy_reset(session_->handle.get());
      }

    private:
      http_session_impl* session_;
      std::string method_;
      std::string url_;
      string_list header_;
      std::unique_ptr<http_reader> reader_;
      http_header_parser parser_;
      std::exception_ptr exception_;

      static size_t read_cb(char* data, size_t size, size_t count, void* self) {
        return static_cast<http_task*>(self)->read(data, size * count);
      }

      static size_t header_cb(char* data, size_t size, size_t count, void* self) {
        return static_cast<http_task*>(self)->header(data, size * count);
      }

      static size_t write_cb(char* data, size_t size, size_t count, void* self) {
        return static_cast<http_task*>(self)->write(data, size * count);
      }

      size_t read(char* data, size_t size) {
        try {
          if (reader_) {
            size_t result = reader_->read(data, size);
            if (session_->progress_cb) {
              if (!session_->progress_cb(reader_->now, reader_->total)) {
                return CURL_READFUNC_ABORT;
              }
            }
            return result;
          }
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return CURL_READFUNC_ABORT;
      }

      size_t header(const char* data, size_t size) {
        try {
          if (session_->header_cb) {
            if (parser_.parse(data, size)) {
              long code = 0;
              check(curl_easy_getinfo(session_->handle.get(), CURLINFO_RESPONSE_CODE, &code));
              if (!session_->header_cb(code, parser_.get())) {
                return 0;
              }
            }
          }
          return size;
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return 0;
      }

      size_t write(const char* data, size_t size) {
        try {
          if (session_->write_cb) {
            if (!session_->write_cb(data, size)) {
              return 0;
            }
          }
          return size;
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return 0;
      }
    };

    void http_session_impl::request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& header,
        http_request_body body,
        const char* data,
        size_t size) {
      http_task task(this, method, url, header, body, data, size);
      task.request();
    }
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
