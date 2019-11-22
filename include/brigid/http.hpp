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
  class http {
  public:
    virtual ~http() = 0;
    virtual void request(const char*, const char*, const std::map<std::string, std::string>&, const char*, size_t) = 0;
  };

  std::unique_ptr<http> make_http(std::function<bool (int, const std::map<std::string, std::string>&)>, std::function<void (const char*, size_t)>);
}

#endif
