// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_HTTP_HPP
#define BRIGID_HTTP_HPP

#include "noncopyable.hpp"

#include <stddef.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace brigid {
  enum class http_request_body { none, data, file };

  class http_initializer : private noncopyable {
  public:
    http_initializer();
    ~http_initializer();
  };

  namespace {
    http_initializer http_initializer_instance;
  }

  void open_http();

  class http_session {
  public:
    virtual ~http_session() = 0;
    virtual bool request(
        const std::string&,
        const std::string&,
        const std::map<std::string, std::string>&,
        http_request_body,
        const char*,
        size_t) = 0;
  };

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (size_t, size_t)>,
      std::function<bool (int, const std::map<std::string, std::string>&)>,
      std::function<bool (const char*, size_t)>,
      bool,
      const std::string&,
      const std::string&);
}

#endif
