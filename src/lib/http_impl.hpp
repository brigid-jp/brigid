// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HTTP_IMPL_HPP
#define BRIGID_HTTP_IMPL_HPP

#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>

#include <stddef.h>
#include <map>
#include <memory>
#include <string>

namespace brigid {
  class http_header_parser : private noncopyable {
  public:
    bool parse(const char*, size_t);
    const std::map<std::string, std::string>& get() const;
  private:
    std::string buffer_;
    std::string field_;
    std::map<std::string, std::string> header_;
    bool parse_impl(const char*, size_t);
    void parse_impl();
  };

  class http_reader {
  public:
    virtual ~http_reader() = 0;
    virtual size_t read(char*, size_t) = 0;
    virtual size_t total() const = 0;
    virtual size_t now() const = 0;
  };

  std::unique_ptr<http_reader> make_http_reader(http_request_body, const char*, size_t);
}

#endif
