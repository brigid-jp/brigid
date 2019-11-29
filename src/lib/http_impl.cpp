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
    bool is_digit(uint8_t c) {
      return 0x30 <= c && c <= 0x39;
    }

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
      return is_digit(c) || (0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A);
    }

    bool is_vchar(uint8_t c) {
      return (0x21 <= c && c <= 0x7E) || 0x80 <= c;
    }
  }

  http_header_parser::http_header_parser()
    : code_() {}

  bool http_header_parser::parse(const char* data, size_t size) {
    buffer_.append(data, size);

    size_t m = 0;
    while (true) {
      size_t n = buffer_.find("\r\n", m);
      if (n == std::string::npos) {
        break;
      }
      if (parse_impl(buffer_.data() + m, n - m)) {
        buffer_.clear();
        return true;
      }
      m = n + 2;
    }

    buffer_ = buffer_.substr(m);
    return false;
  }

  bool http_header_parser::parse_impl(const char* pb, size_t n) {
    const char* pe = pb + n;

    if (pb == pe) {
      if (!field_.empty()) {
        parse_impl();
      }
      field_.clear();
      return true;
    }

    {
      int s = 1;
      int code = 0;
      for (const char* p = pb; pb != pe; ++p) {
        char c = *p;
        switch (s) {
          case  1: s = c == 'H'    ?  2 : 0; break;
          case  2: s = c == 'T'    ?  3 : 0; break;
          case  3: s = c == 'T'    ?  4 : 0; break;
          case  4: s = c == 'P'    ?  5 : 0; break;
          case  5: s = c == '/'    ?  6 : 0; break;
          case  6: s = is_digit(c) ?  7 : 0; break;
          case  7: s = c == '.'    ?  8 : 0; break;
          case  8: s = is_digit(c) ?  9 : 0; break;
          case  9: s = c == ' '    ? 10 : 0; break;
          case 10: s = is_digit(c) ? 11 : 0; code = code * 10 + c - 0x30; break;
          case 11: s = is_digit(c) ? 12 : 0; code = code * 10 + c - 0x30; break;
          case 12: s = is_digit(c) ? 13 : 0; code = code * 10 + c - 0x30; break;
          case 13: s = c == ' '    ? 14 : 0; break;
        }
        if (s == 0 || s == 14) {
          break;
        }
      }
      if (s == 14) {
        code_ = code;
        field_.clear();
        header_.clear();
        return false;
      }
    }

    {
      const char* p = pe;
      const char* q = pe;
      while (p != pb) {
        --p;
        char c = *p;
        if (c == ' ' || c == '\t') {
          q = p;
        } else {
          break;
        }
      }
      pe = q;
    }

    // https://tools.ietf.org/html/rfc7230#section-3.2.4
    //
    // A user agent that receives an obs-fold in a response message that is
    // not within a message/http container MUST replace each received
    // obs-fold with one or more SP octets prior to interpreting the field
    // value.

    if (pb == pe) {
      field_.append(" ");
      return false;
    }

    {
      const char* q = nullptr;
      for (const char* p = pb; p != pe; ++p) {
        char c = *p;
        if (c == ' ' || c == '\t') {
          q = p;
        } else {
          break;
        }
      }
      if (q) {
        field_.append(" ");
        field_.append(q + 1, pe);
        return false;
      }
    }

    if (!field_.empty()) {
      parse_impl();
    }
    field_.assign(pb, pe);
    return false;
  }

  int http_header_parser::code() const {
    return code_;
  }

  const std::map<std::string, std::string>& http_header_parser::get() const {
    return header_;
  }

  void http_header_parser::parse_impl() {
    const char* pb = field_.data();
    const char* pe = pb + field_.size();

    int s = 1;
    const char* q1 = nullptr;
    const char* q2 = nullptr;
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
          if (is_tchar(c)) {
            s = 2;
          } else if (c == ':') {
            s = 3;
            q1 = p;
          } else {
            s = 0;
          }
          break;
        case 3:
          if (c == ' ' || c == '\t') {
            s = 3;
          } else if (is_vchar(c)) {
            s = 4;
            q2 = p;
          } else {
            s = 0;
          }
          break;
        case 4:
          if (is_vchar(c) || c == ' ' || c == '\t') {
            s = 4;
          } else {
            s = 0;
          }
          break;
      }
      if (s == 0) {
        break;
      }
    }

    if (s < 3) {
      throw BRIGID_ERROR("cannot parse header");
    }

    if (q2) {
      header_[std::string(pb, q1)] = std::string(q2, pe);
    } else {
      header_[std::string(pb, q1)] = std::string();
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
