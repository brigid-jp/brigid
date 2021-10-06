// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http_request_parser.hpp"

#include <vector>

namespace brigid {
  namespace {
    %%{
      machine http_request_parser;

      include abnf "abnf.rl";

      main :=
        method
          >{ method_ = q - qs; }
          ${ *q++ = fc; }
          %{ *q++ = '\0'; }
        SP
        request_target
          >{ request_target_ = q - qs; }
          ${ *q++ = fc; }
          %{ *q++ = '\0'; }
        SP
        HTTP_version
          >{ http_version_ = q - qs; }
          ${ *q++ = fc; }
          %{ *q++ = '\0'; }
        CRLF

        (
          field_name
            >{ field_name_ = q - qs; }
            ${ *q++ = fc; }
            %{ *q++ = '\0'; }
          ":"
          OWS
          (
            field_content
              ${ *q++ = fc; }
            |
            obs_fold
              @{ *q++ = ' '; }
          )*
            >{ field_value_ = q - qs; }
            %{ *q++ = '\0'; }
          OWS
          CRLF
            %{ header_fields_.emplace_back(field_name_, field_value_); }
        )*

        CRLF @{ fbreak; };

      write data noerror nofinal noentry;
    }%%
  }

  class http_request_parser::impl {
  public:
    impl() {
      buffer_.reserve(1024);
      header_fields_.reserve(16);
      reset();
    }

    void reset() {
      %%write init;
      buffer_.clear();
      q_ = 0;
      position_ = 0;
      method_ = 0;
      request_target_ = 0;
      http_version_ = 0;
      field_name_ = 0;
      field_value_ = 0;
      header_fields_.clear();
    }

    std::pair<parser_state, const char*> parse(const char* data, size_t size) {
      const char* p = data;
      const char* pe = data + size;

      size_t n = q_ + size;
      if (buffer_.size() < n) {
        buffer_.resize(n);
      }
      char* qs = &buffer_[0];
      char* q = &buffer_[q_];

      %%write exec;

      position_ += p - data;
      q_ = q - qs;

      if (cs == %%{ write error; }%%) {
        return std::make_pair(parser_state::error, p);
      }
      if (cs >= %%{ write first_final; }%%) {
        return std::make_pair(parser_state::accept, p);
      }
      return std::make_pair(parser_state::running, p);
    }

    size_t position() const {
      return position_;
    }

    const char* method() const {
      return &buffer_[method_];
    }

    const char* request_target() const {
      return &buffer_[request_target_];
    }

    const char* http_version() const {
      return &buffer_[http_version_];
    }

    std::pair<const char*, const char*> header_field(size_t i) const {
      if (i < header_fields_.size()) {
        const auto& header_field = header_fields_[i];
        return std::make_pair(&buffer_[header_field.first], &buffer_[header_field.second]);
      } else {
        return std::make_pair(nullptr, nullptr);
      }
    }

  private:
    int cs;
    std::vector<char> buffer_;
    size_t q_;
    size_t position_;
    size_t method_;
    size_t request_target_;
    size_t http_version_;
    size_t field_name_;
    size_t field_value_;
    std::vector<std::pair<size_t, size_t>> header_fields_;
  };

  http_request_parser::http_request_parser()
    : impl_(new impl()) {}

  http_request_parser::~http_request_parser() {}

  void http_request_parser::reset() {
    impl_->reset();
  }

  std::pair<parser_state, const char*> http_request_parser::parse(const char* data, size_t size) {
    return impl_->parse(data, size);
  }

  size_t http_request_parser::position() const {
    return impl_->position();
  }

  const char* http_request_parser::method() const {
    return impl_->method();
  }

  const char* http_request_parser::request_target() const {
    return impl_->request_target();
  }

  const char* http_request_parser::http_version() const {
    return impl_->http_version();
  }

  std::pair<const char*, const char*> http_request_parser::header_field(size_t i) const {
    return impl_->header_field(i);
  }
}
