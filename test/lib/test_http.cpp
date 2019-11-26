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

void test2() {
  int status_code = -1;
  std::string body;
  auto session = brigid::make_http_session(
      [&](int code, const std::map<std::string, std::string>&) -> bool {
        status_code = code;
        return true;
      },
      [&](const char* data, size_t size) -> bool {
        body += std::string(data, size);
        return true;
      },
      [](size_t sent, size_t total, size_t expected) -> bool {
        std::cout << "progress " << sent << ", " << total << ", " << expected << "\n";
        return true;
      });

  std::string data = "foo\nbar\nbaz\nqux\n";
  session->request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", std::map<std::string, std::string>(), brigid::http_request_body::data, data.data(), data.size());
  BRIGID_CHECK(status_code == 201 || status_code == 204);
  BRIGID_CHECK(body.empty());

  session->request("GET", "https://brigid.jp/test/dav/auth-none/test.txt", std::map<std::string, std::string>());
  BRIGID_CHECK(status_code == 200);
  BRIGID_CHECK(body == data);
  body.clear();

  session->request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt", std::map<std::string, std::string>());
  BRIGID_CHECK(status_code == 204);
  BRIGID_CHECK(body.empty());
}

brigid::make_test_case make_test1("http test1", test1);
brigid::make_test_case make_test2("http test2", test2);
