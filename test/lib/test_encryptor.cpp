// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <brigid/crypto.hpp>

#include <algorithm>
#include <vector>

// https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
static const std::string data = "The quick brown fox jumps over the lazy dog";
static const std::string key = "01234567890123456789012345678901";
static const std::string iv = "0123456789012345";
static const std::string expect =
  "\xE0\x6F\x63\xA7\x11\xE8\xB7\xAA\x9F\x94\x40\x10\x7D\x46\x80\xA1"
  "\x17\x99\x43\x80\xEA\x31\xD2\xA2\x99\xB9\x53\x02\xD4\x39\xB9\x70"
  "\x2C\x8E\x65\xA9\x92\x36\xEC\x92\x07\x04\x91\x5C\xF1\xA9\x8A\x44";

void test_encryptor1() {
  std::vector<char> buffer(data.size() + 16);
  brigid::encryptor encryptor("aes-256-cbc", key.data(), key.size(), iv.data(), iv.size());
  size_t result = encryptor.update(data.data(), data.size(), buffer.data(), buffer.size(), true);

  BRIGID_CHECK(result == expect.size());
  buffer.resize(result);
  BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), expect.begin()));
}

brigid::make_test_case make_test_encryptor1("test_encryptor1", test_encryptor1);

void test_encryptor2() {
  std::vector<char> buffer(data.size() + 16);
  brigid::encryptor encryptor("aes-256-cbc", key.data(), key.size(), iv.data(), iv.size());

  size_t result = encryptor.update(data.data(), 16, buffer.data(), buffer.size(), false);
  BRIGID_CHECK(result == expect.size() / 3);

  result = encryptor.update(data.data() + 16, 16, buffer.data() + 16, buffer.size() - 16, false);
  BRIGID_CHECK(result == expect.size() / 3);

  result = encryptor.update(data.data() + 32, data.size() - 32, buffer.data() + 32, buffer.size() - 32, true);
  BRIGID_CHECK(result == expect.size() / 3);

  buffer.resize(expect.size());
  BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), expect.begin()));
}

brigid::make_test_case make_test_encryptor2 { "test_encryptor2", test_encryptor2 };

void test_encryptor3() {
  try {
    brigid::encryptor("no-such-cipher", key.data(), key.size(), iv.data(), iv.size());
  } catch (const std::exception& e) {
    return;
  }
  BRIGID_CHECK(!"unreachable");
}

brigid::make_test_case make_test_encryptor3 { "test_encryptor3", test_encryptor3 };

