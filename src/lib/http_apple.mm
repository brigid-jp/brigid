// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>

namespace brigid {
  namespace {
    class http_session {
    };
  }
}


namespace brigid {
  namespace {
    class http_impl : public http {
    };
  }

  std::unique_ptr<http> make_http(std::function<bool (int, const std::map<std::string, std::string>&)> header_cb, std::function<void (const char*, size_t)> write_cb) {
    throw std::runtime_error("error");
  }
}
