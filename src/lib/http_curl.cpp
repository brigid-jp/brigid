// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>

#include <stdexcept>

namespace brigid {
  std::unique_ptr<http_session> make_http_session() {
    throw std::runtime_error("no impl");
  }
}
