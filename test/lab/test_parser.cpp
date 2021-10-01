// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <exception>
#include <iostream>

namespace brigid {
  namespace {
    void run() {
    }
  }
}

int main(int ac, char* av[]) {
  try {
    brigid::run();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 1;
}
