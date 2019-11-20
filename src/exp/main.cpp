// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

int main(int argc, char* argv[]) {
  std::string url = "http://brigid.jp/love2d-excersise/";

  if (argc > 1) {
    url = argv[1];
  }

  brigid::http(url);
  return 0;
}
