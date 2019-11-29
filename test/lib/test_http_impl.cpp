// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <http_curl.hpp>

#include <vector>

namespace {
  bool parse(brigid::http_header_parser& parser, const std::string& line) {
    return parser.parse(line.data(), line.size());
  }

  std::string get(const brigid::http_header_parser& parser, const std::string& key) {
    auto iterator = parser.get().find(key);
    if (iterator == parser.get().end()) {
      return std::string();
    } else {
      return iterator->second;
    }
  }

  void test1() {
    std::vector<std::string> source {
      "HTTP/1.1 200 OK\r\n",
      "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n",
      "Server: Apache\r\n",
      "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n",
      "ETag: \"34aa387-d-1568eb00\"\r\n",
      "Accept-Ranges: bytes\r\n",
      "Content-Length: 51\r\n",
      "Vary: Accept-Encoding\r\n",
      "Content-Type: text/plain\r\n",
    };

    brigid::http_header_parser parser;
    for (const auto& line : source) {
      BRIGID_CHECK(!parser.parse(line.data(), line.size()));
    }
    BRIGID_CHECK(parse(parser, "\r\n"));

    BRIGID_CHECK(parser.get().size() == source.size() - 1);
    BRIGID_CHECK(get(parser, "Server") == "Apache");
    BRIGID_CHECK(get(parser, "ETag") == "\"34aa387-d-1568eb00\"");
    BRIGID_CHECK(get(parser, "X-No-Such-Header") == "");
  }

  void test2() {
    brigid::http_header_parser parser;
    parse(parser, "HTTP/1.1 200 OK\r\n");
    parse(parser, "X-Test1:foo bar\r\n");
    parse(parser, "X-Test2: foo bar \r\n");
    parse(parser, "X-Test3:  foo  bar  \r\n");
    parse(parser, "X-Test4: foo\r\n");
    parse(parser, "\tbar\r\n");
    parse(parser, "X-Test5: foo \r\n");
    parse(parser, " \r\n");
    parse(parser, "  bar\r\n");
    parse(parser, "X-Test6:\t \t foo bar \t \t\r\n");
    parse(parser, "\r\n");
    BRIGID_CHECK(parser.get().size() == 6);
    BRIGID_CHECK(get(parser, "X-Test1") == "foo bar");
    BRIGID_CHECK(get(parser, "X-Test2") == "foo bar");
    BRIGID_CHECK(get(parser, "X-Test3") == "foo  bar");
    BRIGID_CHECK(get(parser, "X-Test4") == "foo bar");
    BRIGID_CHECK(get(parser, "X-Test5") == "foo  bar");
    BRIGID_CHECK(get(parser, "X-Test6") == "foo bar");

    parse(parser, "HTTP/1.1 200 OK\r\n");
    parse(parser, "\r\n");
    BRIGID_CHECK(parser.get().empty());

    parse(parser, "HTTP/1.1 200 OK\r\n");
    parse(parser, "X-Test1:\r\n");
    parse(parser, "X-Test2:\r\n");
    parse(parser, " 日本語\r\n");
    parse(parser, "\r\n");
    BRIGID_CHECK(parser.get().size() == 2);
    BRIGID_CHECK(get(parser, "X-Test1") == "");
    BRIGID_CHECK(get(parser, "X-Test2") == "日本語");

    parse(parser, "HTTP/1.1 200 OK\r\n");
    parse(parser, "X-Test1: a \x1F\r\n");
    BRIGID_CHECK_THROW([&](){ parse(parser, "\r\n"); });
  }

  BRIGID_MAKE_TEST_CASE(test1);
  BRIGID_MAKE_TEST_CASE(test2);
}
