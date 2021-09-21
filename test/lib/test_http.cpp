// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include <brigid/version.hpp>
#include "test.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace {
  using namespace std::placeholders;

  static const std::map<std::string, std::string> empty_header;

  class test_client {
  public:
    explicit test_client(
        bool credential = false,
        const std::string& username = std::string(),
        const std::string& password = std::string(),
        int cancel = 0)
      : session_(brigid::make_http_session(
            std::bind(&test_client::progress_cb, this, _1, _2),
            std::bind(&test_client::header_cb, this, _1, _2),
            std::bind(&test_client::write_cb, this, _1, _2),
            credential,
            username,
            password)),
        cancel_(cancel),
        cancel_count_(),
        progress_count_(),
        header_count_(),
        write_count_(),
        code_(),
        canceled_() {}

    int request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& header = empty_header,
        brigid::http_request_body body = brigid::http_request_body::none,
        const char* data = nullptr,
        size_t size = 0) {
      cancel_count_ = 0;
      progress_count_ = 0;
      header_count_ = 0;
      write_count_ = 0;
      code_ = 0;
      canceled_ = !session_->request(method, url, header, body, data, size);
      body_ = out_.str();
      out_.str(std::string());
      out_.clear();
      return code_;
    }

    size_t cancel_count() const {
      return cancel_count_;
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

    bool canceled() const {
      return canceled_;
    }

  private:
    std::unique_ptr<brigid::http_session> session_;
    int cancel_;
    size_t cancel_count_;
    size_t progress_count_;
    size_t header_count_;
    size_t write_count_;
    int code_;
    std::map<std::string, std::string> header_;
    std::ostringstream out_;
    std::string body_;
    bool canceled_;

    bool progress_cb(size_t now, size_t total) {
      std::cout << "[debug] progress " << now << "/" << total << "\n";

      if (cancel_ == 1) {
        if (now * 2 >= total) {
          ++cancel_count_;
          std::cout << "explicitly cancel in the progress callback\n";
          return false;
        }
      }

      ++progress_count_;
      std::cout << "progress " << now << "/" << total << "\n";
      return true;
    }

    bool header_cb(int code, const std::map<std::string, std::string>& header) {
      if (cancel_ == 2) {
        ++cancel_count_;
        std::cout << "explicitly cancel in the header callback\n";
        return false;
      }

      ++header_count_;
      code_ = code;
      header_ = header;
      std::cout << "header " << code_ << "\n";
      return true;
    }

    bool write_cb(const char* data, size_t size) {
      if (cancel_ == 3) {
        ++cancel_count_;
        std::cout << "explicitly cancel in the writer callback\n";
        return false;
      }

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
    BRIGID_CHECK(client.header_count() == 1);
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
    BRIGID_CHECK(client.header_count() == 1);
    BRIGID_CHECK(client.code() == 201 || client.code() == 204);
    if (client.code() == 204) {
      BRIGID_CHECK(client.write_count() == 0);
      BRIGID_CHECK(client.body().empty());
    }

    client.request("GET", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.header_count() == 1);
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
    std::string data;
    for (size_t i = 0; i < 4096; i += 16) {
      data += "0123456789ABCDE\n";
    }
    {
      std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
      for (size_t i = 0; i < 1024 * 1024; i += data.size()) {
        out << data;
      }
    }

    test_client client;
    client.request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", empty_header, brigid::http_request_body::file, filename.data(), filename.size());
    BRIGID_CHECK(client.progress_count() > 0);
    BRIGID_CHECK(client.code() == 201 || client.code() == 204);

    std::cout << "progress_count " << client.progress_count() << " " << 1024 * 1024 / client.progress_count() << "\n";

    client.request("GET", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body().size() == 1024 * 1024);

    std::cout << "write_count " << client.write_count() << " " << 1024 * 1024 / client.write_count() << "\n";

    client.request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.code()== 204);
    BRIGID_CHECK(client.body().empty());

    remove(filename.c_str());
  }

  void test4() {
    test_client client;
    client.request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=1");
    BRIGID_CHECK(client.header_count() == 1);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == "ok\n");
    BRIGID_CHECK(client.header("Location") == "");

    client.request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=11");
    BRIGID_CHECK(client.header_count() == 1);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == "ok\n");
    BRIGID_CHECK(client.header("Location") == "");

    BRIGID_CHECK_THROW([&](){ client.request("GET", "https://brigid.jp/test/cgi/redirect.cgi?count=22"); });
  }

  void test5() {
    {
      test_client client;
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);

      client.request("GET", "https://brigid.jp/test/dav/auth-basic/");
      BRIGID_CHECK(client.code() == 401);
    }

    {
      test_client client(true, "brigid", "O6jIOchrWCGuOSB4");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);

      client.request("GET", "https://brigid.jp/test/dav/auth-basic/");
      BRIGID_CHECK(client.code() == 200);
    }
  }

  void test6() {
    {
      test_client client;
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);

      client.request("GET", "https://brigid.jp/test/dav/auth-digest/");
      BRIGID_CHECK(client.code() == 401);
    }

    {
      test_client client(true, "brigid", "YlrMTunTORZvrgSt");
      client.request("GET", "https://brigid.jp/test/dav/auth-none/");
      BRIGID_CHECK(client.code() == 200);

      client.request("GET", "https://brigid.jp/test/dav/auth-digest/");
      BRIGID_CHECK(client.code() == 200);
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

    client.request("POST", "https://brigid.jp/test/cgi/cat.cgi", header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body().empty());

    client.request("POST", "https://brigid.jp/test/cgi/env.cgi", header, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body().find("CONTENT_TYPE=application/x-www-form-urlencoded\n") != std::string::npos);
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
    BRIGID_CHECK(client.body().find("QUERY_STRING=%20%21\n") != std::string::npos);
  }

  void test14() {
    test_client client;
    client.request("GET", "https://brigid.jp/test/cgi/cookie.cgi");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.header("Set-Cookie") == "code=42; Path=/test/cgi/; Secure");
    BRIGID_CHECK(client.body() == "HTTP_COOKIE=\n");

    client.request("GET", "https://brigid.jp/test/cgi/cookie.cgi");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.header("Set-Cookie") == "code=42; Path=/test/cgi/; Secure");
    BRIGID_CHECK(client.body() == "HTTP_COOKIE=\n");

    std::map<std::string, std::string> header {
      { "Cookie", "code=69" },
    };
    client.request("GET", "https://brigid.jp/test/cgi/cookie.cgi", header);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.header("Set-Cookie") == "");
    BRIGID_CHECK(client.body() == "HTTP_COOKIE=code=69\n");
  }

  void test15() {
    test_client client;

    std::string ua = std::string("brigid/") + brigid::get_version();
    std::map<std::string, std::string> header {
      { "User-Agent", ua },
    };

    client.request("GET", "https://brigid.jp/test/cgi/env.cgi", header);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body().find("USER_AGENT=" + ua + "\n") != std::string::npos);
  }

  void test16() {
    std::string filename = "test.dat";
    std::string data;
    for (size_t i = 0; i < 4096; i += 16) {
      data += "0123456789ABCDE\n";
    }
    {
      std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
      for (size_t i = 0; i < 8 * 1024 * 1024; i += data.size()) {
        out << data;
      }
    }

    test_client client(false, "", "", 1);;
    client.request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", empty_header, brigid::http_request_body::file, filename.data(), filename.size());
    std::cout << "[debug] progress_count " << client.progress_count() << "\n";
    BRIGID_CHECK(client.progress_count() > 0);
    std::cout << "[debug] cancel_count " << client.cancel_count() << "\n";
    BRIGID_CHECK(client.cancel_count() == 1);
    BRIGID_CHECK(client.code() == 0);
    BRIGID_CHECK(client.canceled());

    remove(filename.c_str());
  }

  void test17() {
    test_client client(false, "", "", 2);
    client.request("GET", "https://brigid.jp/test/cgi/env.cgi");
    BRIGID_CHECK(client.cancel_count() == 1);
    BRIGID_CHECK(client.code() == 0);
    BRIGID_CHECK(client.canceled());
  }

  void test18() {
    test_client client(false, "", "", 3);
    client.request("GET", "https://brigid.jp/test/cgi/env.cgi");
    BRIGID_CHECK(client.cancel_count() == 1);
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.canceled());
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
  BRIGID_MAKE_TEST_CASE(test14);
  BRIGID_MAKE_TEST_CASE(test15);
  BRIGID_MAKE_TEST_CASE(test16);
  BRIGID_MAKE_TEST_CASE(test17);
  BRIGID_MAKE_TEST_CASE(test18);
}
