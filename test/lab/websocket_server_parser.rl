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

      request_line_ =
        method
          ${ std::cout << "method [" << fc << "]\n"; }
        SP
        request_target
          ${ std::cout << "request_target [" << fc << "]\n"; }
        SP
        HTTP_version
          ${ std::cout << "HTTP_version [" << fc << "]\n"; }
        CRLF;

      main := request_line_ @{ fbreak; };
    }%%

    %%write data;
  }

  class websocket_server_parser::impl {
  public:
    impl() {
      %%write init;
    }

    void update(const char* data, const char* end) {
      const char* p = data;
      const char* pe = nullptr;

      %%write exec;

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
  };

  websocket_server_parser::websocket_server_parser()
    : impl_(new impl()) {}

  websocket_server_parser::~websocket_server_parser() {}

  void websocket_server_parser::update(const char* data, size_t size) {
    impl_->update(data, nullptr);
  }
}
