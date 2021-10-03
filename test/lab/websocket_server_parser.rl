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

      # https://datatracker.ietf.org/doc/html/rfc3986#appendix-A

      unreserved = alpha | digit | "-" | "." | "_" | "~";
      pct_encoded = "%" xdigit xdigit;
      sub_delims
        = "!" | "$" | "&" | "'" | "(" | ")"
        | "*" | "+" | "," | ";" | "=";

      pchar = unreserved | pct_encoded | sub_delims | ":" | "@";

      segment = pchar*;
      segment_nz = pchar+;
      segment_nz_nc = (unreserved | pct_encoded | sub_delims | "@");

      # https://datatracker.ietf.org/doc/html/rfc7230#appendix-B
      DIGIT = digit;
      ALPHA = alpha;

      tchar
        = "!" | "#" | "$" | "%" | "&" | "'" | "*" | "+" | "-" | "."
        | "^" | "_" | "`" | "|" | "~" | DIGIT | ALPHA;

      dec_octet
        = digit
        | [1-9] digit
        | "1" digit{2}
        | "2" [0-4] digit
        | "25" [0-5];

      IPv4Address = dec_octet "." dec_octet "." dec_octet "." dec_octet;

      h16 = xdigit{1,4};
      ls32 = h16 ":" h16 | IPv4Address;

      IPv6Address
        = (h16 ":"){6} ls32
        | "::" (h16 ":"){5} ls32
        | h16? "::" (h16 ":"){4} ls32
        | ((h16 ":")? h16)? "::" (h16 ":"){3} ls32
        | ((h16 ":"){0,2} h16)? "::" (h16 ":"){2} ls32
        | ((h16 ":"){0,3} h16)? "::" (h16 ":")? ls32
        | ((h16 ":"){0,4} h16)? "::" ls32
        | ((h16 ":"){0,5} h16)? "::" h16
        | ((h16 ":"){0,6} h16)? "::";

      IPvFuture = "v" xdigit+ "." (unreserved | sub_delims | ":")+;

      token = tchar+;
      absolute_path = ("/" segment)+;
      query = (pchar | "/" | "?")*;
      IP_Literal = "[" (IPv6Address | IPvFuture) "]";
      reg_name = (unreserved | pct_encoded | sub_delims)*;

      userinfo = (unreserved | pct_encoded | sub_delims | ":")*;
      host = IP_Literal | IPv4Address | reg_name;
      port = digit*;
      authority = (userinfo "@")? host (":" port)?;

      path_abempty = ("/" segment)*;
      path_absolute = "/" (segment_nz* ("/" segment)*)?;
      path_rootless = segment_nz ("/" segment)*;
      path_empty = pchar*;

      scheme = alpha (alpha | digit | "+" | "-" ".")*;
      hier_part
        = "//" authority path_abempty
        | path_absolute
        | path_rootless
        | path_empty;

      absolute_URI = scheme ":" hier_part ("?" query)?;

      origin_form = absolute_path ("?" query)?;
      absolute_form = absolute_URI;
      authority_form = authority;
      asterisk_form = "*";

      method = token;
      request_target
        = origin_form
        | absolute_form
        | authority_form
        | asterisk_form;

      HTTP_name = "HTTP";
      HTTP_version = HTTP_name "/" digit "." digit;

      SP = " ";
      CRLF = "\r\n";

      request_line =
        method
          ${ std::cout << "method [" << fc << "]\n"; }
        SP
        request_target
          ${ std::cout << "request_target [" << fc << "]\n"; }
        SP
        HTTP_version
          ${ std::cout << "HTTP_version [" << fc << "]\n"; }
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
