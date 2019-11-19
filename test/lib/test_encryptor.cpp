// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"

#include <stddef.h>
#include <assert.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <brigid/crypto.hpp>

static const std::string data { "The quick brown fox jumps over the lazy dog" };
static const std::string key { "01234567890123456789012345678901" };
static const std::string iv { "0123456789012345" };

static const char* expect_data =
  "\xE0\x6F\x63\xA7\x11\xE8\xB7\xAA\x9F\x94\x40\x10\x7D\x46\x80\xA1"
  "\x17\x99\x43\x80\xEA\x31\xD2\xA2\x99\xB9\x53\x02\xD4\x39\xB9\x70"
  "\x2C\x8E\x65\xA9\x92\x36\xEC\x92\x07\x04\x91\x5C\xF1\xA9\x8A\x44";
static const size_t expect_size = 48;

void test1() {
  std::vector<char> buffer(data.size() + 16);
  brigid::encryptor encryptor { "aes-256-cbc", key.data(), key.size(), iv.data(), iv.size() };
  size_t result = encryptor.update(data.data(), data.size(), buffer.data(), buffer.size(), true);

  BRIGID_CHECK(result == expect_size);
  buffer.resize(result);
  BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), expect_data));
}

brigid::test_case add_test1 { "test1", test1 };

void test2() {
  std::vector<char> buffer(data.size() + 16);
  brigid::encryptor encryptor { "aes-256-cbc", key.data(), key.size(), iv.data(), iv.size() };

  size_t result = 0;

  size_t i = 0;
  size_t j = 0;
  for (; i < data.size(); i += 16) {
    j += result;
  }

  result = encryptor.update(data.data(), 16, buffer.data(), buffer.size(), false);
  BRIGID_CHECK(result == 16);

  result = encryptor.update(data.data() + 16, 16, buffer.data() + 16, buffer.size() - 16, false);
  BRIGID_CHECK(result == 16);

  result = encryptor.update(data.data() + 32, data.size() - 32, buffer.data() + 32, buffer.size() - 32, true);
  BRIGID_CHECK(result == 16);

  buffer.resize(48);
  BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), expect_data));
}

brigid::test_case add_test2 { "test2", test2 };
