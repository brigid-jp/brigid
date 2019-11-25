// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <brigid/http.hpp>

#include <iostream>
#include <string>

void test1() {
  std::string body;
  auto session = brigid::make_http_session(
      [](int code, const std::map<std::string, std::string>& headers) -> bool {
        BRIGID_CHECK(code == 200);
        BRIGID_CHECK(headers.find("Content-Length")->second == "0");
        BRIGID_CHECK(headers.find("Content-Type")->second == "text/html; charset=UTF-8");
        return true;
      },
      [&](const char* data, size_t size) -> bool {
        body += std::string(data, size);
        return true;
      },
      nullptr);

  session->request("GET", "https://brigid.jp/", std::map<std::string, std::string>());
  BRIGID_CHECK(body.empty());
}

brigid::make_test_case make_test1("http test (1)", test1);
