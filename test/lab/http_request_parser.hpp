// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HTTP_REQUEST_PARSER_HPP
#define BRIGID_HTTP_REQUEST_PARSER_HPP

#include <brigid/noncopyable.hpp>

#include <stddef.h>
#include <memory>
#include <utility>

namespace brigid {
  enum class parser_state { running, accept, error };

  class http_request_parser : private noncopyable {
  public:
    http_request_parser();
    ~http_request_parser();
    void reset();
    std::pair<parser_state, const char*> parse(const char*, size_t);
    size_t position() const;
    const char* method() const;
    const char* request_target() const;
    const char* http_version() const;
    std::pair<const char*, const char*> header_field(size_t) const;
  private:
    class impl;
    std::unique_ptr<impl> impl_;
  };
}

#endif
