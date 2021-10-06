// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http_request_parser.hpp"

#include <vector>

#include <string.h>

namespace brigid {
  namespace {
    %%{
      machine http_request_parser;

      include abnf "abnf.rl";

      main :=
        method
          >{ ps = p; method = q; }
          # ${ *q++ = fc; }
          # %{ *q++ = '\0'; }
          %{ n = p - ps; memcpy(q, ps, n); q += n; *q++ = '\0'; }
        SP
        # request_target
        [^ ]+
          >{ ps = p; request_target = q; }
          # ${ *q++ = fc; }
          # %{ *q++ = '\0'; }
          %{ n = p - ps; memcpy(q, ps, n); q += n; *q++ = '\0'; }
        SP
        HTTP_version
          >{ ps = p; http_version = q; }
          # ${ *q++ = fc; }
          # %{ *q++ = '\0'; }
          %{ n = p - ps; memcpy(q, ps, n); q += n; *q++ = '\0'; }
        CRLF

        (
          field_name
            >{ ps = p; field_name = q; }
            # ${ *q++ = fc; }
            # %{ *q++ = '\0'; }
            %{ n = p - ps; memcpy(q, ps, n); q += n; *q++ = '\0'; }
          ":"
          OWS
          (
            field_content
              # ${ *q++ = fc; }
            |
            obs_fold
              # ${ *q++ = ' '; }
          )*
            >{ ps = p; field_value = q; }
            # %{ *q++ = '\0'; }
            %{ n = p - ps; memcpy(q, ps, n); q += n; *q++ = '\0'; }
          OWS
          CRLF
            %{
              // const auto result = header_fields.emplace(field_name, field_value);
              // if (!result.second) {
              //   result.first->second.append(", ").append(field_value);
              // }
              // field_name.clear();
              // field_value.clear();
            }
        )*

        CRLF @{ fbreak; };

      write data noerror nofinal noentry;
    }%%
  }

  class http_request_parser::impl {
  public:
    impl() : position(), buffer(4096) {
      q = buffer.data();
      %%write init;
    }

    void clear() {
      q = buffer.data();
      %%write init;
    }

    std::pair<parser_state, const char*> parse(const char* data, size_t size) {
      const char* p = data;
      const char* pe = data + size;

      %%write exec;

      position += p - data;
      if (cs == %%{ write error; }%%) {
        return std::make_pair(parser_state::error, p);
      }
      if (cs >= %%{ write first_final; }%%) {
        return std::make_pair(parser_state::accept, p);
      }
      return std::make_pair(parser_state::running, p);
    }

    std::pair<parser_state, const char*> parse_(const char* data, size_t size) {
      // memcpy(buffer.data(), data, size);

      for (size_t i = 0; i < size; ++i) {
        buffer[i] = data[i];
      }

      return std::make_pair(parser_state::accept, data + size);
    }

    int cs;
    size_t position;
    std::vector<char> buffer;
    const char* ps;
    char* q;
    size_t n;

    char* method;
    char* request_target;
    char* http_version;
    char* field_name;
    char* field_value;

    // std::string method;
    // std::string request_target;
    // std::string http_version;
    // std::string field_name;
    // std::string field_value;
    // std::map<std::string, std::string> header_fields;
  };

  http_request_parser::http_request_parser()
    : impl_(new impl()) {}

  http_request_parser::~http_request_parser() {}

  void http_request_parser::clear() {
    impl_->clear();
  }

  std::pair<parser_state, const char*> http_request_parser::parse(const char* data, size_t size) {
    return impl_->parse(data, size);
  }

  size_t http_request_parser::position() const {
    return impl_->position;
  }

  std::string http_request_parser::method() const {
    return impl_->method;
  }

  std::string http_request_parser::request_target() const {
    return impl_->request_target;
  }

  std::string http_request_parser::http_version() const {
    return impl_->http_version;
  }

  std::map<std::string, std::string> http_request_parser::header_fields() const {
    std::map<std::string, std::string> header_fields;
    return header_fields;
  }
}
