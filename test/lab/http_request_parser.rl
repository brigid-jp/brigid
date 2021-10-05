// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http_request_parser.hpp"

namespace brigid {
  namespace {
    %%{
      machine http_request_parser;

      include abnf "abnf.rl";

      main :=
        method
          ${ method += fc; }
        SP
        request_target
          ${ request_target += fc; }
        SP
        HTTP_version
          ${ http_version += fc; }
        CRLF

        (
          field_name
            ${ field_name += fc; }
          ":"
          OWS
          (
            field_content
              ${ field_value += fc; }
            |
            obs_fold
              @{ field_value += ' '; }
          )*
          OWS
          CRLF
            %{
              const auto result = header_fields.emplace(field_name, field_value);
              if (!result.second) {
                result.first->second.append(", ").append(field_value);
              }
              field_name.clear();
              field_value.clear();
            }
        )*

        CRLF @{ fbreak; };
    }%%

    %%write data;
  }

  class http_request_parser::impl {
  public:
    impl() : position(), line(), column() {
      %%write init;
    }

    std::pair<parser_state, const char*> parse(const char* data, size_t size) {
      const char* p = data;
      const char* pe = data + size;

      %%write exec;

      position += p - data;

      if (cs == http_request_parser_error) {
        return std::make_pair(parser_state::error, p);
      }
      if (cs >= http_request_parser_first_final) {
        return std::make_pair(parser_state::accept, p);
      }
      return std::make_pair(parser_state::running, p);
    }

    int cs;
    size_t position;
    size_t line;
    size_t column;
    std::string method;
    std::string request_target;
    std::string http_version;
    std::string field_name;
    std::string field_value;
    std::map<std::string, std::string> header_fields;
  };

  http_request_parser::http_request_parser()
    : impl_(new impl()) {}

  http_request_parser::~http_request_parser() {}

  std::pair<parser_state, const char*> http_request_parser::parse(const char* data, size_t size) {
    return impl_->parse(data, size);
  }

  size_t http_request_parser::position() const {
    return impl_->position;
  }

  size_t http_request_parser::line() const {
    return impl_->column;
  }

  size_t http_request_parser::column() const {
    return impl_->column;
  }

  const std::string& http_request_parser::method() const {
    return impl_->method;
  }

  const std::string& http_request_parser::request_target() const {
    return impl_->request_target;
  }

  const std::string& http_request_parser::http_version() const {
    return impl_->http_version;
  }

  const std::map<std::string, std::string>& http_request_parser::header_fields() const {
    return impl_->header_fields;
  }
}
