// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"
#include <brigid/crypto.hpp>

#include <algorithm>
#include <vector>

// https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
static const std::string plain = "The quick brown fox jumps over the lazy dog";
static const std::string key128 = "0123456789012345";
static const std::string key192 = "012345678901234567890123";
static const std::string key256 = "01234567890123456789012345678901";
static const std::string iv = "0123456789012345";
static const std::string expect128(
    "\x30\x89\xE6\xBC\x22\x4B\xD9\x5B\x85\xCF\x56\xF4\xB9\x67\x11\x8A"
    "\xAA\x47\x05\x43\x0F\x25\xB6\xB4\xD9\x53\x18\x8A\xD1\x5D\xD7\x8F"
    "\x38\x67\x57\x7E\x7D\x58\xE1\x8C\x9C\xB3\x40\x64\x7C\x8B\x4F\xD8", 48);
static const std::string expect192(
    "\x70\xEE\xD7\x34\x63\x1F\xFF\x2A\x7E\x00\xB1\x70\x7A\xED\x19\xBB"
    "\xA9\x51\x20\x8B\x7F\xF1\x2F\x28\xD0\x43\xC8\x6C\x52\x06\x2C\x3E"
    "\x3F\xD6\xC1\x54\x8E\x4E\x79\x84\x05\xD0\x39\xD0\x46\x3F\x1C\x15", 48);
static const std::string expect256(
    "\xE0\x6F\x63\xA7\x11\xE8\xB7\xAA\x9F\x94\x40\x10\x7D\x46\x80\xA1"
    "\x17\x99\x43\x80\xEA\x31\xD2\xA2\x99\xB9\x53\x02\xD4\x39\xB9\x70"
    "\x2C\x8E\x65\xA9\x92\x36\xEC\x92\x07\x04\x91\x5C\xF1\xA9\x8A\x44", 48);

void test1(const std::string& cipher, const std::string& key, const std::string& expect) {
  std::vector<char> buffer(plain.size() + 16);
  brigid::encryptor encryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
  size_t result = encryptor.update(plain.data(), plain.size(), buffer.data(), buffer.size(), true);

  BRIGID_CHECK(result == expect.size());
  buffer.resize(result);
  BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), expect.begin()));
}

void test2(const std::string& cipher, const std::string& key, const std::string& expect) {
  std::vector<char> buffer(plain.size() + 16);
  brigid::encryptor encryptor(cipher, key.data(), key.size(), iv.data(), iv.size());

  size_t result = encryptor.update(plain.data(), 16, buffer.data(), buffer.size(), false);
  BRIGID_CHECK(result == expect.size() / 3);

  result = encryptor.update(plain.data() + 16, 16, buffer.data() + 16, buffer.size() - 16, false);
  BRIGID_CHECK(result == expect.size() / 3);

  result = encryptor.update(plain.data() + 32, plain.size() - 32, buffer.data() + 32, buffer.size() - 32, true);
  BRIGID_CHECK(result == expect.size() / 3);

  buffer.resize(expect.size());
  BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), expect.begin()));
}

void test_no_such_cipher() {
  try {
    brigid::encryptor("no-such-cipher", nullptr, 0, nullptr, 0);
  } catch (const std::exception& e) {
    return;
  }
  BRIGID_CHECK(!"unreachable");
}

brigid::make_test_case make_test1_128("encryptor aes-128-cbc (1)", [](){ test1("aes-128-cbc", key128, expect128); });
brigid::make_test_case make_test2_128("encryptor aes-128-cbc (2)", [](){ test2("aes-128-cbc", key128, expect128); });
brigid::make_test_case make_test1_192("encryptor aes-192-cbc (1)", [](){ test1("aes-192-cbc", key192, expect192); });
brigid::make_test_case make_test2_("encryptor aes-192-cbc (2)", [](){ test2("aes-192-cbc", key192, expect192); });
brigid::make_test_case make_test1_256("encryptor aes-256-cbc (1)", [](){ test1("aes-256-cbc", key256, expect256); });
brigid::make_test_case make_test2_256("encryptor aes-256-cbc (2)", [](){ test2("aes-256-cbc", key256, expect256); });
brigid::make_test_case make_test_no_such_cipher("encryptor no-such-cipher", &test_no_such_cipher);
