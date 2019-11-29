// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "error.hpp"
#include "http_impl.hpp"

#include <stdio.h>
#include <string.h>
#include <algorithm>

namespace brigid {
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
      // https://tools.ietf.org/html/rfc7230#section-3.2.4
      //
      // A user agent that receives an obs-fold in a response message that is
      // not within a message/http container MUST replace each received
      // obs-fold with one or more SP octets prior to interpreting the field
      // value.
      buffer_.append(" ");
      if (p3 < p2) {
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

  http_reader::~http_reader() {}

  namespace {
    FILE* check(FILE* handle) {
      if (!handle) {
        throw BRIGID_ERROR("cannot fopen", errno);
      }
      return handle;
    }

    using file_t = std::unique_ptr<FILE, decltype(&fclose)>;

    file_t make_file(FILE* handle) {
      return file_t(handle, &fclose);
    }

    class http_reader_impl : public http_reader {
    public:
      http_reader_impl()
        : total_(),
          now_() {}

      virtual size_t total() const {
        return total_;
      }

      virtual size_t now() const {
        return now_;
      }

    protected:
      void set_total(size_t total) {
        total_ = total;
      }

      void add_now(size_t now) {
        now_ += now;
      }

    private:
      size_t total_;
      size_t now_;
    };

    class http_data_reader : public http_reader_impl, private noncopyable {
    public:
      http_data_reader(const char* data, size_t size)
        : data_(data),
          size_(size) {
        set_total(size);
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = std::min(size, size_);
        memmove(data, data_, result);
        data_ += result;
        size_ -= result;
        add_now(result);
        return result;
      }

    private:
      const char* data_;
      size_t size_;
    };

    class http_file_reader : public http_reader_impl, private noncopyable {
    public:
      http_file_reader(const char* path)
        : handle_(make_file(check(fopen(path, "rb")))) {
        fseek(handle_.get(), 0, SEEK_END);
        set_total(ftell(handle_.get()));
        fseek(handle_.get(), 0, SEEK_SET);
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = fread(data, 1, size, handle_.get());
        add_now(result);
        return result;
      }

    private:
      file_t handle_;
    };

  }

  std::unique_ptr<http_reader> make_http_reader(http_request_body body, const char* data, size_t size) {
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
}
