// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include "http_curl.hpp"

#include <curl/curl.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <vector>

namespace brigid {
  namespace {
    int http_initializer_counter = 0;
  }

  http_initializer::http_initializer() {
    if (++http_initializer_counter == 1) {
      // how to check error?
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
      headers_.clear();
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
      throw std::runtime_error("parser error");
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
    return headers_;
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

    if (!p1 || !p2 || p3) {
      throw std::runtime_error("parser error");
    }

    headers_[std::string(pb, p1)] = std::string(p2, pe);
  }

  namespace {
    void check(CURLcode code) {
      if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
      }
    }

    CURL* check(CURL* result) {
      if (!result) {
        check(CURLE_FAILED_INIT);
      }
      return result;
    }

    using easy_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

    easy_t make_easy(CURL* easy) {
      return easy_t(easy, &curl_easy_cleanup);
    }

    class string_list {
    public:
      string_list()
        : slist_() {}

      ~string_list() {
        curl_slist_free_all(slist_);
      }

      void append(const std::string& value) {
        if (curl_slist* slist = curl_slist_append(slist_, value.c_str())) {
          slist_ = slist;
        } else {
          throw std::runtime_error("slist error");
        }
      }

      curl_slist* get() const {
        return slist_;
      }

    private:
      curl_slist* slist_;
    };

    class http_session_impl : public http_session {
    public:
      friend class http_task;

      http_session_impl()
        : handle_(make_easy(check(curl_easy_init()))),
          credential_() {}

      virtual void set_progress_cb(std::function<bool (size_t, size_t)> progress_cb) {
        progress_cb_ = progress_cb;
      }

      virtual void set_header_cb(std::function<bool (int, const std::map<std::string, std::string>&)> header_cb) {
        header_cb_ = header_cb;
      }

      virtual void set_write_cb(std::function<bool (const char*, size_t)> write_cb) {
        write_cb_ = write_cb;
      }

      virtual void set_credential() {
        credential_ = false;
        username_.clear();
        password_.clear();
      }

      virtual void set_credential(const std::string& username, const std::string& password) {
        credential_ = true;
        username_ = username;
        password_ = password;
      }

      virtual void request(const std::string&, const std::string&, const std::map<std::string, std::string>&, http_request_body, const char*, size_t);

    private:
      easy_t handle_;
      std::function<bool (size_t, size_t)> progress_cb_;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
      bool credential_;
      std::string username_;
      std::string password_;
    };

    class http_reader {
    public:
      virtual ~http_reader() {}
      virtual size_t read(char*, size_t) = 0;
      virtual size_t now() const = 0;
      virtual size_t total() const = 0;
    };

    class http_data_reader : public http_reader {
    public:
      http_data_reader(const char* data, size_t size)
        : data_(data),
          size_(size),
          now_(),
          total_(size){}

      virtual size_t read(char* data, size_t size) {
        size_t result = std::min(size, size_);
        memmove(data, data_, result);
        data_ += result;
        size_ -= result;
        now_ += result;
        return result;
      }

      virtual size_t now() const {
        return now_;
      }

      virtual size_t total() const {
        return total_;
      }

    private:
      const char* data_;
      size_t size_;
      size_t now_;
      size_t total_;
    };

    class http_file_reader : public http_reader {
    public:
      http_file_reader(const char* path)
        : handle_(fopen(path, "rb")),
          now_(),
          total_() {
        if (!handle_) {
          throw std::runtime_error("fopen error");
        }

        fseek(handle_, 0, SEEK_END);
        total_ = ftell(handle_);
        fseek(handle_, 0, SEEK_SET);
      }

      virtual ~http_file_reader() {
        if (handle_) {
          fclose(handle_);
        }
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = fread(data, 1, size, handle_);
        now_ += result;
        return result;
      }

      virtual size_t now() const {
        return now_;
      }

      virtual size_t total() const {
        return total_;
      }

    private:
      FILE* handle_;
      size_t now_;
      size_t total_;
    };

    class http_task {
    public:
      http_task(http_session_impl* session, std::unique_ptr<http_reader>&& reader, const std::map<std::string, std::string>& header)
        : session_(session),
          reader_(std::move(reader)),
          header_() {
        for (const auto& field : header) {
          append(field.first + ": " + field.second);
        }
      }

      void request(const std::string& method, const std::string& url) {
        CURL* handle = session_->handle_.get();

        check(curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1));
        check(curl_easy_setopt(handle, CURLOPT_VERBOSE, 1));

        check(curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, method.c_str()));
        if (method == "HEAD") {
          check(curl_easy_setopt(handle, CURLOPT_NOBODY, 1));
        }
        check(curl_easy_setopt(handle, CURLOPT_URL, url.c_str()));

        if (header_) {
          check(curl_easy_setopt(handle, CURLOPT_HTTPHEADER, header_));
        }

        if (reader_) {
          check(curl_easy_setopt(handle, CURLOPT_UPLOAD, 1));
          check(curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, reader_->total()));
          check(curl_easy_setopt(handle, CURLOPT_READFUNCTION, &http_task::read_cb));
          check(curl_easy_setopt(handle, CURLOPT_READDATA, this));
        }

        if (session_->header_cb_) {
          check(curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, &http_task::header_cb));
          check(curl_easy_setopt(handle, CURLOPT_HEADERDATA, this));
        }

        if (session_->write_cb_) {
          check(curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &http_task::write_cb));
          check(curl_easy_setopt(handle, CURLOPT_WRITEDATA, this));
        }

        if (session_->credential_) {
          check(curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY));
          check(curl_easy_setopt(handle, CURLOPT_USERNAME, session_->username_.c_str()));
          check(curl_easy_setopt(handle, CURLOPT_PASSWORD, session_->password_.c_str()));
        }

        CURLcode code = curl_easy_perform(handle);

        if (exception_) {
          std::rethrow_exception(exception_);
        }
        check(code);
      }

      ~http_task() {
        curl_easy_reset(session_->handle_.get());
        curl_slist_free_all(header_);
      }

    private:
      http_session_impl* session_;
      std::unique_ptr<http_reader> reader_;
      curl_slist* header_;
      http_header_parser parser_;
      std::exception_ptr exception_;

      void append(const std::string& value) {
        if (curl_slist* header = curl_slist_append(header_, value.c_str())) {
          header_ = header;
        } else {
          throw std::runtime_error("slist error");
        }
      }

      static size_t read_cb(char* data, size_t m, size_t n, void* self) {
        return static_cast<http_task*>(self)->read(data, m * n);
      }

      static size_t header_cb(char* data, size_t m, size_t n, void* self) {
        return static_cast<http_task*>(self)->header(data, m * n);
      }

      static size_t write_cb(char* data, size_t m, size_t n, void* self) {
        return static_cast<http_task*>(self)->write(data, m * n);
      }

      size_t read(char* data, size_t size) {
        try {
          if (reader_) {
            size_t result = reader_->read(data, size);
            if (session_->progress_cb_) {
              if (!session_->progress_cb_(reader_->now(), reader_->total())) {
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
          if (session_->header_cb_) {
            if (parser_.parse(data, size)) {
              long code = 0;
              check(curl_easy_getinfo(session_->handle_.get(), CURLINFO_RESPONSE_CODE, &code));
              if (!session_->header_cb_(code, parser_.get())) {
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
          if (session_->write_cb_) {
            if (!session_->write_cb_(data, size)) {
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
        const std::map<std::string, std::string>& headers,
        http_request_body body,
        const char* data,
        size_t size) {

      std::unique_ptr<http_reader> reader;
      switch (body) {
        case http_request_body::data:
          if (data) {
            reader.reset(new http_data_reader(data, size));
          }
          break;
        case http_request_body::file:
          reader.reset(new http_file_reader(data));
          break;
      }

      http_task task(this, move(reader), headers);
      task.request(method, url);
    }
  }

  std::unique_ptr<http_session> make_http_session() {
    return std::unique_ptr<http_session>(new http_session_impl());
  }
}
