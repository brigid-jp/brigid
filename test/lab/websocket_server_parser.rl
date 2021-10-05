// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "websocket_server_parser.hpp"

#include <iostream>
#include <map>

namespace brigid {
  namespace {
    %%{
      machine websocket;

      include abnf "abnf.rl";

      main :=
        method
          ${ method_ += fc; }
        SP
        request_target
          ${ request_target_ += fc; }
        SP
        HTTP_version
          ${ http_version_ += fc; }
        CRLF

        (
          field_name ${ field_name_ += fc; }
          ":" OWS
          (
            field_content ${ field_value_ += fc; }
            |
            obs_fold @{ field_value_ += ' '; }
          )*
          OWS

          CRLF
            %{
              auto result = header_fields_.emplace(field_name_, field_value_);
              if (!result.second) {
                result.first->second.append(", ").append(field_value_);
              }
              field_name_.clear();
              field_value_.clear();
            }
        )*

        CRLF @{ fbreak; };
    }%%

    %%write data;
  }

  class websocket_server_parser::impl {
  public:
    impl() {
      %%write init;
    }

    void update(const char* data, size_t size) {
      const char* p = data;
      const char* pe = data + size;

      if (cs >= websocket_first_final) {
        return;
      }

      %%write exec;
      std::cout << "cs " << cs << " index " << (p - data) << "\n";

      if (cs == websocket_error) {
        std::cerr
          << "ERROR\n"
          << "[" << std::string(p, size - (p - data)) << "]\n";
        return;
      }

      if (cs < websocket_first_final) {
        return;
      }

      std::cerr << "FINAL\n";

      std::cout
        << method_ << "\n"
        << request_target_ << "\n"
        << http_version_ << "\n";

      for (auto kv : header_fields_) {
        std::cout << "[" << kv.first << "]=>[" << kv.second << "]\n";
      }
      std::cout << "\n";
    }

  private:
    int cs;
    std::string method_;
    std::string request_target_;
    std::string http_version_;
    std::string field_name_;
    std::string field_value_;
    std::map<std::string, std::string> header_fields_;
  };

  websocket_server_parser::websocket_server_parser()
    : impl_(new impl()) {}

  websocket_server_parser::~websocket_server_parser() {}

  void websocket_server_parser::update(const char* data, size_t size) {
    impl_->update(data, size);
  }
}
