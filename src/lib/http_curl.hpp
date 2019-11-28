// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HTTP_CURL_HPP
#define BRIGID_HTTP_CURL_HPP

#include <map>
#include <string>

namespace brigid {
  class http_header_parser {
    http_header_parser(const http_header_parser&) = delete;
    http_header_parser& operator=(const http_header_parser&) = delete;
  public:
    http_header_parser();
    bool parse(const char*, size_t);
    const std::map<std::string, std::string>& get() const;
  private:
    int state_;
    std::string buffer_;
    std::map<std::string, std::string> header_;
    void parse_impl();
  };
}

#endif
