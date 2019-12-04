// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <brigid/http.hpp>

#include <stdio.h>
#include <fstream>
#include <iostream>

namespace {
  using namespace std::placeholders;

  static const std::map<std::string, std::string> empty_header;

  class test_client {
  public:
    explicit test_client(
        brigid::http_authentication_scheme auth_scheme = brigid::http_authentication_scheme::none,
        const std::string& username = std::string(),
        const std::string& password = std::string())
      : session_(brigid::make_http_session(
            std::bind(&test_client::progress_cb, this, _1, _2),
            std::bind(&test_client::header_cb, this, _1, _2),
            std::bind(&test_client::write_cb, this, _1, _2),
            auth_scheme,
            username,
            password)),
        progress_count_(),
        header_count_(),
        write_count_(),
        code_() {
    }

    int request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& header = empty_header,
        brigid::http_request_body body = brigid::http_request_body::none,
        const char* data = nullptr,
        size_t size = 0) {
      progress_count_ = 0;
      header_count_ = 0;
      write_count_ = 0;
      code_ = 0;
      session_->request(method, url, header, body, data, size);
      body_ = out_.str();
      out_.str(std::string());
      out_.clear();
      return code_;
    }

    size_t progress_count() const {
      return progress_count_;
    }

    size_t header_count() const {
      return header_count_;
    }

    size_t write_count() const {
      return write_count_;
    }

    int code() const {
      return code_;
    }

    const std::map<std::string, std::string>& header() const {
      return header_;
    }

    std::string header(const std::string& key) const {
      auto iterator = header_.find(key);
      if (iterator == header_.end()) {
        return std::string();
      } else {
        return iterator->second;
      }
    }

    const std::string& body() const {
      return body_;
    }

  private:
    std::unique_ptr<brigid::http_session> session_;
    size_t progress_count_;
    size_t header_count_;
    size_t write_count_;
    int code_;
    std::map<std::string, std::string> header_;
    std::ostringstream out_;
    std::string body_;

    bool progress_cb(size_t now, size_t total) {
      ++progress_count_;
      std::cout << "progress " << now << "/" << total << "\n";
      return true;
    }

    bool header_cb(int code, const std::map<std::string, std::string>& header) {
      ++header_count_;
      code_ = code;
      header_ = header;
      std::cout << "header " << code_ << "\n";
      return true;
    }

    bool write_cb(const char* data, size_t size) {
      ++write_count_;
      out_.write(data, size);
      return true;
    }
  };

  void test1() {
    test_client client;
    client.request("GET", "https://brigid.jp/");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.write_count() == 0);
    BRIGID_CHECK(client.header("Content-Length") == "0");
    BRIGID_CHECK(client.header("Content-Type") == "text/html; charset=UTF-8");
    BRIGID_CHECK(client.body().empty());
  }

  void test2() {
    static const std::string data = "foo\nbar\nbaz\nqux\n";

    test_client client;
    client.request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", empty_header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.progress_count() > 0);
    BRIGID_CHECK(client.code() == 201 || client.code() == 204);
    if (client.code() == 204) {
      BRIGID_CHECK(client.write_count() == 0);
      BRIGID_CHECK(client.body().empty());
    }

    client.request("GET", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.write_count() > 0);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == data);

    client.request("HEAD", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.write_count() == 0);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body().empty());

    client.request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.write_count() == 0);
    BRIGID_CHECK(client.code()== 204);
    BRIGID_CHECK(client.body().empty());

    client.request("GET", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.write_count() > 0);
    BRIGID_CHECK(client.code() == 404);

    client.request("HEAD", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.write_count() == 0);
    BRIGID_CHECK(client.code() == 404);
    BRIGID_CHECK(client.body().empty());
  }

  void test3() {
    std::string filename = "test.dat";
    {
      std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
      for (size_t i = 0; i < 1024 * 1024 / 16; ++i) {
        out << "0123456789ABCDE\n";
      }
    }

    test_client client;
    client.request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", empty_header, brigid::http_request_body::file, filename.data(), filename.size());
    BRIGID_CHECK(client.progress_count() > 0);
    BRIGID_CHECK(client.code() == 201 || client.code() == 204);
    std::cout << "[" << client.body() << "]\n";

    client.request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.code()== 204);
    BRIGID_CHECK(client.body().empty());

    remove(filename.c_str());
  }

  void test4() {
    test_client client;
    client.request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=1");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == "ok\n");
    BRIGID_CHECK(client.header("Location") == "");

    // default redirection
    // WinHTTP: 10
    client.request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=10");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == "ok\n");
    BRIGID_CHECK(client.header("Location") == "");
  }

  void test5() {
    {
      test_client client;
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);
      // std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-basic/");
      BRIGID_CHECK(client.code() == 401);
      // std::cout << "[" << client.body() << "]\n";
    }

    {
      test_client client(brigid::http_authentication_scheme::basic, "brigid", "O6jIOchrWCGuOSB4");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);
      // std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-basic/");
      BRIGID_CHECK(client.code() == 200);
      // std::cout << "[" << client.body() << "]\n";
    }

    {
      test_client client(brigid::http_authentication_scheme::digest, "brigid", "O6jIOchrWCGuOSB4");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);
      // std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-basic/");
      BRIGID_CHECK(client.code() == 401);
      // std::cout << "[" << client.body() << "]\n";
    }

    {
      test_client client(brigid::http_authentication_scheme::any, "brigid", "O6jIOchrWCGuOSB4");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);
      // std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-basic/");
      BRIGID_CHECK(client.code() == 200);
      // std::cout << "[" << client.body() << "]\n";
    }
  }

  void test6() {
    std::map<std::string, std::string> header {
      { "Depth", "1" },
    };

    {
      test_client client;
      client.request("GET", "https://brigid.jp/test/dav/auth-none/", header);
      BRIGID_CHECK(client.code() == 200);
      std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-digest/", header);
      BRIGID_CHECK(client.code() == 401);
      std::cout << "[" << client.body() << "]\n";
    }

    {
      test_client client(brigid::http_authentication_scheme::basic, "brigid", "YlrMTunTORZvrgSt");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/", header);
      BRIGID_CHECK(client.code() == 200);
      std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-digest/", header);
      BRIGID_CHECK(client.code() == 401);
      std::cout << "[" << client.body() << "]\n";
    }

    {
      test_client client(brigid::http_authentication_scheme::digest, "brigid", "YlrMTunTORZvrgSt");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/", header);
      BRIGID_CHECK(client.code() == 200);
      std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-digest/", header);
      BRIGID_CHECK(client.code() == 200);
      std::cout << "[" << client.body() << "]\n";
    }

    {
      test_client client(brigid::http_authentication_scheme::any, "brigid", "YlrMTunTORZvrgSt");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/", header);
      BRIGID_CHECK(client.code() == 200);
      std::cout << "[" << client.body() << "]\n";

      client.request("GET", "https://brigid.jp/test/dav/auth-digest/", header);
      BRIGID_CHECK(client.code() == 200);
      std::cout << "[" << client.body() << "]\n";
    }
  }

  void test7() {
    static const std::string data = R"({"foo":42})";

    test_client client;
    std::map<std::string, std::string> header {
      { "Content-Type", "application/json; charset=UTF-8" },
    };

    client.request("POST", "https://brigid.jp/test/cgi/cat.cgi", header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == data);

    client.request("POST", "https://brigid.jp/test/cgi/env.cgi", header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    std::cout << "[" << client.body() << "]\n";
    BRIGID_CHECK(client.body().find("CONTENT_TYPE=application/json; charset=UTF-8\n") != std::string::npos);
  }

  void test8() {
    test_client client;
    BRIGID_CHECK_THROW([&](){ client.request("GET", "https://133.242.153.239/"); });
  }

  void test9() {
    test_client client;
    client.request("GET", "https://brigid.jp/test/cgi/nph-header.cgi");
    BRIGID_CHECK(client.code() == 200);

    for (const auto& field : client.header()) {
      std::cout << "[" << field.first << "]=[" << field.second << "]\n";
    }

    auto check = [](const std::string& value) -> bool {
      size_t n = value.size();
      return n > 6
          && value.substr(0, 3) == "foo"
          && value.substr(3, n - 6).find_first_not_of(" \t") == std::string::npos
          && value.substr(n - 3, 3) == "bar";
    };

    BRIGID_CHECK(client.header().size() == 11);
    BRIGID_CHECK(client.header("X-Test1") == "foo bar");
    BRIGID_CHECK(client.header("X-Test2") == "foo bar");
    BRIGID_CHECK(client.header("X-Test3") == "foo  bar");
    BRIGID_CHECK(check(client.header("X-Test4")));
    BRIGID_CHECK(check(client.header("X-Test5")));
    BRIGID_CHECK(client.header("X-Test6") == "foo bar");
    BRIGID_CHECK(client.header("x-test7") == "foo bar");
    BRIGID_CHECK(client.header("X-tEsT8") == "foo bar");
  }

  void test10() {
    static const std::string data = "";

    test_client client;
    std::map<std::string, std::string> header {
      { "Content-Type", "application/x-www-form-urlencoded" },
    };

    client.request("POST", "https://brigid.jp/test/cgi/env.cgi", header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    std::cout << "[" << client.body() << "]\n";
    BRIGID_CHECK(client.body().find("CONTENT_TYPE=application/x-www-form-urlencoded\n") != std::string::npos);

    client.request("POST", "https://brigid.jp/test/cgi/cat.cgi", header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    std::cout << "[" << client.body() << "]\n";
  }

  void test11() {
    test_client client;
    BRIGID_CHECK_THROW([&](){ client.request("GET", "https://no-such-host.brigid.jp/"); });
  }

  void test12() {
    test_client client;
    BRIGID_CHECK_THROW([&](){ client.request("GET", "!!! invalid url !!!"); });
  }

  void test13() {
    test_client client;
    client.request("GET", "https://brigid.jp/test/%63%67%69/env.cgi?%20%21");
    BRIGID_CHECK(client.code() == 200);
    std::cout << "[" << client.body() << "]\n";
    BRIGID_CHECK(client.body().find("QUERY_STRING=%20%21\n") != std::string::npos);
  }

  BRIGID_MAKE_TEST_CASE(test1);
  BRIGID_MAKE_TEST_CASE(test2);
  BRIGID_MAKE_TEST_CASE(test3);
  BRIGID_MAKE_TEST_CASE(test4);
  BRIGID_MAKE_TEST_CASE(test5);
  BRIGID_MAKE_TEST_CASE(test6);
  BRIGID_MAKE_TEST_CASE(test7);
  BRIGID_MAKE_TEST_CASE(test8);
  BRIGID_MAKE_TEST_CASE(test9);
  BRIGID_MAKE_TEST_CASE(test10);
  BRIGID_MAKE_TEST_CASE(test11);
  BRIGID_MAKE_TEST_CASE(test12);
  BRIGID_MAKE_TEST_CASE(test13);
}
