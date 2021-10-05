// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HTTP_REQUEST_PARSER_HPP
#define BRIGID_HTTP_REQUEST_PARSER_HPP

#include <brigid/noncopyable.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>

#include <stddef.h>

namespace brigid {
  enum class parser_state { running, accept, error };

  class http_request_parser : private noncopyable {
  public:
    http_request_parser();
    ~http_request_parser();
    std::pair<parser_state, const char*> parse(const char* data, size_t size);
    size_t position() const;
    size_t line() const;
    size_t column() const;
    const std::string& method() const;
    const std::string& request_target() const;
    const std::string& http_version() const;
    const std::map<std::string, std::string>& header_fields() const;

  private:
    class impl;
    std::unique_ptr<impl> impl_;
  };
}

#endif
