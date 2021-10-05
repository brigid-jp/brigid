// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "websocket_server_parser.hpp"

#include <iostream>

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
          # field_name
          #   ${ field_name_ += fc; }
          # ":" OWS field_value
          #   ${ field_value_ += fc; }
          #   %{
          #     std::cout << "[" << field_name_ << "] => [" << field_value_ << "]\n";
          #     field_name_.clear();
          #     field_value_.clear();
          #   }
          # OWS
          (
            field_name
              # ${
              #   switch (fc) {
              #     case '\r': std::cout << "\\r"; break;
              #     case '\n': std::cout << "\\n"; break;
              #     default: std::cout << fc;
              #   }
              # }
              # %{ std::cout << "%\n"; }

            ":"
            OWS
            (
              field_content
                ${
                  switch (fc) {
                    case '\r': std::cout << "\\r"; break;
                    case '\n': std::cout << "\\n"; break;
                    default: std::cout << fc;
                  }
                }
              # |
              # obs_fold
              #   @{ std::cout << "obs_fold\n"; }
            )*
            OWS
              @{ std::cout << "(OWS@)\n"; }
          )

          # header_field
          # (header_field -- "\r\n")
          # [^\r\n]+
            # >{
            #   std::cout << ">";
            #   switch (fc) {
            #     case '\r': std::cout << "\\r"; break;
            #     case '\n': std::cout << "\\n"; break;
            #     default: std::cout << fc << "";
            #   }
            # }

          CRLF
            @{ std::cout << "CRLF\n"; }
        )*

        CRLF @{
          std::cout << "fbreak\n";
          fbreak;
        }

        ;
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

      %%write exec;

      std::cout
        << method_ << "\n"
        << request_target_ << "\n"
        << http_version_ << "\n";

      std::cout << "cs " << cs << "\n";
      std::cout << "index " << (p - data) << "\n";

      if (cs == websocket_error) {
        std::cerr << "ERROR\n";
      }

      if (cs >= websocket_first_final) {
        std::cerr << "FINAL\n";
      }

    }

  private:
    int cs;
    std::string method_;
    std::string request_target_;
    std::string http_version_;
    std::string field_name_;
    std::string field_value_;
  };

  websocket_server_parser::websocket_server_parser()
    : impl_(new impl()) {}

  websocket_server_parser::~websocket_server_parser() {}

  void websocket_server_parser::update(const char* data, size_t size) {
    impl_->update(data, size);
  }
}
