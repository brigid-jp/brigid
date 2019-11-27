// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <brigid/http.hpp>

#include <stdio.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace {
  static const std::map<std::string, std::string> empty_headers;

  class test_client {
  public:
    test_client()
      : session_(brigid::make_http_session()),
        progress_count_(),
        header_count_(),
        write_count_(),
        code_() {
      using namespace std::placeholders;
      session_->set_progress_cb(std::bind(&test_client::progress_cb, this, _1, _2));
      session_->set_header_cb(std::bind(&test_client::header_cb, this, _1, _2));
      session_->set_write_cb(std::bind(&test_client::write_cb, this, _1, _2));
    }

    void set_credential() {
      session_->set_credential();
    }

    void set_credential(const std::string& username, const std::string& password) {
      session_->set_credential(username, password);
    }

    int request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& headers = empty_headers,
        brigid::http_request_body body = brigid::http_request_body::data,
        const char* data = nullptr,
        size_t size = 0) {
      progress_count_ = 0;
      header_count_ = 0;
      write_count_ = 0;
      code_ = 0;
      session_->request(method, url, headers, body, data, size);
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

    std::string header(const std::string& key) const {
      auto iterator = headers_.find(key);
      if (iterator == headers_.end()) {
        return std::string();
      } else {
        return iterator->second;
      }
    }

    const std::map<std::string, std::string>& headers() const {
      return headers_;
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
    std::map<std::string, std::string> headers_;
    std::ostringstream out_;
    std::string body_;

    bool progress_cb(size_t now, size_t total) {
      ++progress_count_;
      std::cout << "progress " << now << "/" << total << "\n";
      return true;
    }

    bool header_cb(int code, const std::map<std::string, std::string>& headers) {
      ++header_count_;
      code_ = code;
      headers_ = headers;
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
    client.request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", empty_headers, brigid::http_request_body::data, data.data(), data.size());
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
    client.request("PUT", "https://brigid.jp/test/dav/auth-none/test.txt", empty_headers, brigid::http_request_body::file, filename.data(), filename.size());
    BRIGID_CHECK(client.progress_count() > 0);
    BRIGID_CHECK(client.code() == 201 || client.code() == 204);
    std::cout << "[" << client.body() << "]\n";

    client.request("DELETE", "https://brigid.jp/test/dav/auth-none/test.txt");
    BRIGID_CHECK(client.progress_count() == 0);
    BRIGID_CHECK(client.code()== 204);
    BRIGID_CHECK(client.body().empty());

    remove("test.dat");
  }

  void test4() {
    test_client client;
    client.request("GET", "https://brigid.jp/test/lua/redirect.lua?count=1");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == "ok\n");
    BRIGID_CHECK(client.header("Location") == "");

    client.request("GET", "https://brigid.jp/test/lua/redirect.lua?count=16");
    BRIGID_CHECK(client.code() == 200);
    BRIGID_CHECK(client.body() == "ok\n");
    BRIGID_CHECK(client.header("Location") == "");
  }

  void test5() {
    test_client client;
    std::map<std::string, std::string> headers {
      { "Depth", "1" },
    };
    client.request("PROPFIND", "https://brigid.jp/test/dav/auth-none/", headers);
    BRIGID_CHECK(client.code() == 207);
    std::cout << "[" << client.body() << "]\n";

    client.request("PROPFIND", "https://brigid.jp/test/dav/auth-basic/", headers);
    BRIGID_CHECK(client.code() == 401);
    std::cout << "[" << client.body() << "]\n";

    client.set_credential("brigid", "O6jIOchrWCGuOSB4");
    client.request("PROPFIND", "https://brigid.jp/test/dav/auth-basic/", headers);
    BRIGID_CHECK(client.code() == 207);
    std::cout << "[" << client.body() << "]\n";
  }

  void test6() {
    test_client client;
    std::map<std::string, std::string> headers {
      { "Depth", "1" },
    };

    // curl does not clear state.authproblem

    // client.set_credential();
    // client.request("PROPFIND", "https://brigid.jp/test/dav/auth-digest/", headers);
    // BRIGID_CHECK(client.code() == 401);
    // std::cout << "[" << client.body() << "]\n";

    client.set_credential("brigid", "YlrMTunTORZvrgSt");
    client.request("PROPFIND", "https://brigid.jp/test/dav/auth-digest/", headers);
    BRIGID_CHECK(client.code() == 207);
    std::cout << "[" << client.body() << "]\n";
  }

  void test7() {
    static const std::string data = R"({"foo":42})";

    test_client client;
    std::map<std::string, std::string> headers {
      { "Content-Type", "application/json; charset=UTF-8" },
    };

    client.request("POST", "https://brigid.jp/test/lua/echo.lua?keys=Content-Type,Content-Length,Expect", headers, brigid::http_request_body::data, data.data(), data.size());
    BRIGID_CHECK(client.code() == 200);
    std::cout << "[" << client.body() << "]\n";
  }

  brigid::make_test_case make_test1("http test1", test1);
  brigid::make_test_case make_test2("http test2", test2);
  brigid::make_test_case make_test3("http test3", test3);
  brigid::make_test_case make_test4("http test4", test4);
  brigid::make_test_case make_test5("http test5", test5);
  brigid::make_test_case make_test6("http test6", test6);
  brigid::make_test_case make_test7("http test7", test7);
}
