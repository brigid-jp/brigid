// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <iostream>
#include <sstream>
#include <thread>

namespace brigid {
  void debug(int key, const std::string& message) {
    std::ostringstream out;
    out << key << " " << std::this_thread::get_id() << " " << message << "\n";
    std::cout << out.str();
  }

  void http(int key, const std::string& url) {
    debug(key, "start");
    debug(key, "done");
  }
}
