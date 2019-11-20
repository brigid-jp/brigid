// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <iostream>
#include <thread>

void thread_function(int key, const std::string& url) {
  brigid::debug(key, "start");
  brigid::http(key, url);
  brigid::debug(key, "finish");
}

int main(int argc, char* argv[]) {
  std::string url = "https://brigid.jp/love2d-excersise/";

  if (argc > 1) {
    url = argv[1];
  }

  brigid::debug(0, "main start");

  std::thread t1(thread_function, 1, url);
  std::thread t2(thread_function, 2, url);
  std::thread t3(thread_function, 3, url);
  std::thread t4(thread_function, 4, url);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  brigid::debug(0, "main finish");

  return 0;
}
