// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/noncopyable.hpp>

#include <memory>

#include <stddef.h>

namespace brigid {
  class websocket_request_parser : private noncopyable {
  public:
    websocket_request_parser();
    ~websocket_request_parser();
    void update(const char* data, size_t size);
  private:
    class impl;
    std::unique_ptr<impl> impl_;
  };
}
