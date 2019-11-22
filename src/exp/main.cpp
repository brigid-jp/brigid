// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void thread_function(int key, const std::string& url) {
  try {
    brigid::debug(key, "start");
    brigid::http(key, url);
    brigid::debug(key, "finish");
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
  }
}

int main(int argc, char* argv[]) {
  int thread = 0;
  std::string url = "https://brigid.jp/love2d-excersise/";

  if (argc > 1) {
    thread = std::stoi(argv[1]);
  }
  if (argc > 2) {
    url = argv[2];
  }

  brigid::debug(0, "main start");

  if (thread <= 0) {
    thread_function(0, url);
  } else {
    std::vector<std::thread> threads;

    for (int i = 0; i < thread; ++i) {
      threads.emplace_back(thread_function, i + 1, url);
    }

    for (int i = 0; i < thread; ++i) {
      threads[i].join();
    }
  }

  brigid::debug(0, "main finish");

  return 0;
}
