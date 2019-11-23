// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>

namespace brigid {
  namespace {
  }
}


namespace brigid {
  namespace {
    class http_session_impl : public http_session {
    };
  }


  std::unique_ptr<http_session> make_http_session(
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      std::function<bool (size_t, size_t, size_t)> read_cb) {
    throw std::runtime_error("error");
  }
}
