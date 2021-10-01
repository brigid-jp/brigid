// vim: syntax=ragel:

#include "websocket_server_parser.hpp"

#include <iostream>

namespace brigid {
  namespace {
    %%{
      machine websocket;

      # https://tools.ietf.org/html/rfc2616

      CHAR = ascii;
      CTL = cntrl | 127;
      SP = ' ';
      HT = '\t';
      CRLF = "\r\n";

      separators
        = "(" | ")" | "<" | ">" | "@"
        | "," | ";" | ":" | "\\" | "\""
        | "/" | "[" | "]" | "?" | "="
        | "{" | "}" | SP | HT;

      token = (CHAR - CTL - separators)+;

      Request_URI = [^ ]+;
      HTTP_Version =
        "HTTP/"
        digit+
          >{ std::cout << "major version enter " << fc << "\n"; }
          @{ std::cout << "major version " << fc << "\n"; }
        "."
        digit+
          >{ std::cout << "minor version enter " << fc << "\n"; }
          @{ std::cout << "minor version " << fc << "\n"; }
        ;

      request_line =
        token @{ std::cout << "tchar " << fc << "\n"; }
        SP Request_URI
        SP HTTP_Version
        CRLF;

      main := request_line @{ fbreak; };
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
