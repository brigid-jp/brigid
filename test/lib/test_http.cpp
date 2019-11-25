// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <brigid/http.hpp>

#include <iostream>
#include <stdexcept>

void test1() {

  int count = 0;
  auto session = brigid::make_http_session(
      [&](int code, const std::map<std::string, std::string>& headers) -> bool {
        std::cout << "code " << code << "\n";
        for (const auto& header : headers) {
          std::cout << header.first << ": " << header.second << "\n";
        }
        if (++count == 2) {
          throw std::runtime_error("foo bar baz");
        }
        return true;
      },
      [](const char* data, size_t size) -> bool {
        std::cout << std::string(data, size);
        return true;
      });

  session->request("GET", "https://brigid.jp/", std::map<std::string, std::string>());
  try {
    session->request("GET", "https://133.242.153.239/", std::map<std::string, std::string>());
  } catch (const std::exception& e) {
    std::cout << "caught " << e.what() << "\n";
  }
  try {
    session->request("GET", "https://brigid.jp/love2d-excersise/", std::map<std::string, std::string>());
  } catch (const std::exception& e) {
    std::cout << "caught " << e.what() << "\n";
  }
  try {
    session->request("GET", "https://brigid.jp/love2d-excersise/", std::map<std::string, std::string>());
  } catch (const std::exception& e) {
    std::cout << "caught " << e.what() << "\n";
  }
}

brigid::make_test_case make_test1("http test (1)", test1);
