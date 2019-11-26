// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HTTP_HPP
#define BRIGID_HTTP_HPP

#include <stddef.h>
#include <map>
#include <memory>
#include <functional>
#include <string>

namespace brigid {
  enum class http_request_body { data, file };

  class http_session {
  public:
    virtual ~http_session() = 0;
    virtual void set_progress_cb(std::function<bool (size_t, size_t)>) = 0;
    virtual void set_header_cb(std::function<bool (int, const std::map<std::string, std::string>&)>) = 0;
    virtual void set_write_cb(std::function<bool (const char*, size_t)>) = 0;

    virtual void request(
        const std::string&,
        const std::string&,
        const std::map<std::string, std::string>&,
        http_request_body,
        const char*,
        size_t) = 0;
  };

  std::unique_ptr<http_session> make_http_session();
}

#endif
