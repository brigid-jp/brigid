// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include "test.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace {
  static const std::string plaintext = "The quick brown fox jumps over the lazy dog";
  static const std::string key128 = "0123456789012345";
  static const std::string key192 = "012345678901234567890123";
  static const std::string key256 = "01234567890123456789012345678901";
  static const std::string iv = "0123456789012345";
  static const char ciphertext128_data[] = {
    '\x30', '\x89', '\xE6', '\xBC', '\x22', '\x4B', '\xD9', '\x5B',
    '\x85', '\xCF', '\x56', '\xF4', '\xB9', '\x67', '\x11', '\x8A',
    '\xAA', '\x47', '\x05', '\x43', '\x0F', '\x25', '\xB6', '\xB4',
    '\xD9', '\x53', '\x18', '\x8A', '\xD1', '\x5D', '\xD7', '\x8F',
    '\x38', '\x67', '\x57', '\x7E', '\x7D', '\x58', '\xE1', '\x8C',
    '\x9C', '\xB3', '\x40', '\x64', '\x7C', '\x8B', '\x4F', '\xD8',
  };
  static const std::string ciphertext128(ciphertext128_data, 48);
  static const char ciphertext192_data[] = {
    '\x70', '\xEE', '\xD7', '\x34', '\x63', '\x1F', '\xFF', '\x2A',
    '\x7E', '\x00', '\xB1', '\x70', '\x7A', '\xED', '\x19', '\xBB',
    '\xA9', '\x51', '\x20', '\x8B', '\x7F', '\xF1', '\x2F', '\x28',
    '\xD0', '\x43', '\xC8', '\x6C', '\x52', '\x06', '\x2C', '\x3E',
    '\x3F', '\xD6', '\xC1', '\x54', '\x8E', '\x4E', '\x79', '\x84',
    '\x05', '\xD0', '\x39', '\xD0', '\x46', '\x3F', '\x1C', '\x15',
  };
  static const std::string ciphertext192(ciphertext192_data, 48);
  static const char ciphertext256_data[] = {
    '\xE0', '\x6F', '\x63', '\xA7', '\x11', '\xE8', '\xB7', '\xAA',
    '\x9F', '\x94', '\x40', '\x10', '\x7D', '\x46', '\x80', '\xA1',
    '\x17', '\x99', '\x43', '\x80', '\xEA', '\x31', '\xD2', '\xA2',
    '\x99', '\xB9', '\x53', '\x02', '\xD4', '\x39', '\xB9', '\x70',
    '\x2C', '\x8E', '\x65', '\xA9', '\x92', '\x36', '\xEC', '\x92',
    '\x07', '\x04', '\x91', '\x5C', '\xF1', '\xA9', '\x8A', '\x44',
  };
  static const std::string ciphertext256(ciphertext256_data, 48);

  void test_encryptor1(brigid::crypto_cipher cipher, const std::string& key, const std::string& ciphertext) {
    std::vector<char> buffer(plaintext.size() + 16);
    auto encryptor = brigid::make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
    size_t result = encryptor->update(plaintext.data(), plaintext.size(), buffer.data(), buffer.size(), true);

    BRIGID_CHECK(result == ciphertext.size());
    buffer.resize(result);
    BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), ciphertext.begin()));
  }

  void test_encryptor2(brigid::crypto_cipher cipher, const std::string& key, const std::string& ciphertext) {
    std::vector<char> buffer(plaintext.size() + 16);
    auto encryptor = brigid::make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size());

    size_t result = encryptor->update(plaintext.data(), 16, buffer.data(), buffer.size(), false);
    BRIGID_CHECK(result == ciphertext.size() / 3);

    result = encryptor->update(plaintext.data() + 16, 16, buffer.data() + 16, buffer.size() - 16, false);
    BRIGID_CHECK(result == ciphertext.size() / 3);

    result = encryptor->update(plaintext.data() + 32, plaintext.size() - 32, buffer.data() + 32, buffer.size() - 32, true);
    BRIGID_CHECK(result == ciphertext.size() / 3);

    buffer.resize(ciphertext.size());
    BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), ciphertext.begin()));
  }

  void test_encryptor3(brigid::crypto_cipher cipher, const std::string& key, const std::string& ciphertext) {
    auto encryptor = brigid::make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
    std::vector<char> buffer(encryptor->calculate_buffer_size(plaintext.size()));

    size_t p = 0;
    for (size_t i = 0; i < plaintext.size(); ++i) {
      size_t result = encryptor->update(plaintext.data() + i, 1, buffer.data() + p, buffer.size() - p, i == plaintext.size() - 1);
      std::cout << i << ", " << result << "\n";
      p += result;
    }

    BRIGID_CHECK(p == ciphertext.size());
    buffer.resize(p);
    BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), ciphertext.begin()));
  }

  void test_decryptor1(brigid::crypto_cipher cipher, const std::string& key, const std::string& ciphertext) {
    std::vector<char> buffer(ciphertext.size());
    auto decryptor = brigid::make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
    size_t result = decryptor->update(ciphertext.data(), ciphertext.size(), buffer.data(), buffer.size(), true);

    BRIGID_CHECK(result == plaintext.size());
    buffer.resize(result);
    BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), plaintext.begin()));
  }

  void test_decryptor2(brigid::crypto_cipher cipher, const std::string& key, const std::string& ciphertext) {
    std::vector<char> buffer(ciphertext.size());
    auto decryptor = brigid::make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size());

    size_t result1 = decryptor->update(ciphertext.data(), 16, buffer.data(), buffer.size(), false);
    size_t result2 = result1 + decryptor->update(ciphertext.data() + 16, 16, buffer.data() + result1, buffer.size() - result1, false);
    size_t result3 = result2 + decryptor->update(ciphertext.data() + 32, 16, buffer.data() + result2, buffer.size() - result2, true);
    BRIGID_CHECK(result3 == plaintext.size());

    std::cout << result1 << ", " << result2 << ", " << result3 << "\n";

    buffer.resize(result3);
    BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), plaintext.begin()));
  }

  void test_decryptor3(brigid::crypto_cipher cipher, const std::string& key, const std::string& ciphertext) {
    auto decryptor = brigid::make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size());
    std::vector<char> buffer(decryptor->calculate_buffer_size(ciphertext.size()));

    size_t p = 0;
    for (size_t i = 0; i < ciphertext.size(); ++i) {
      size_t result = decryptor->update(ciphertext.data() + i, 1, buffer.data() + p, buffer.size() - p, i == ciphertext.size() - 1);
      std::cout << i << ", " << result << "\n";
      p += result;
    }

    BRIGID_CHECK(p == plaintext.size());
    buffer.resize(p);
    BRIGID_CHECK(std::equal(buffer.begin(), buffer.end(), plaintext.begin()));
  }

  void test_error1() {
    BRIGID_CHECK_THROW([&](){ brigid::make_encryptor(brigid::crypto_cipher::aes_256_cbc, key256.data(), key256.size() - 1, iv.data(), iv.size()); });
  }

  void test_error2() {
    BRIGID_CHECK_THROW([&](){ brigid::make_encryptor(brigid::crypto_cipher::aes_256_cbc, key256.data(), key256.size(), iv.data(), iv.size() - 1); });
  }

  void test_buffer_size() {
    auto encryptor = brigid::make_encryptor(brigid::crypto_cipher::aes_256_cbc, key256.data(), key256.size(), iv.data(), iv.size());
    BRIGID_CHECK(encryptor->calculate_buffer_size(17) == 33);
    auto decryptor = brigid::make_decryptor(brigid::crypto_cipher::aes_256_cbc, key256.data(), key256.size(), iv.data(), iv.size());
    BRIGID_CHECK(decryptor->calculate_buffer_size(32) == 32);
  }

  void test_sha256_empty() {
    static const char expect_data[] = {
      '\xE3', '\xB0', '\xC4', '\x42', '\x98', '\xFC', '\x1C', '\x14',
      '\x9A', '\xFB', '\xF4', '\xC8', '\x99', '\x6F', '\xB9', '\x24',
      '\x27', '\xAE', '\x41', '\xE4', '\x64', '\x9B', '\x93', '\x4C',
      '\xA4', '\x95', '\x99', '\x1B', '\x78', '\x52', '\xB8', '\x55',
    };
    static const std::string expect(expect_data, 32);

    auto hasher = brigid::make_hasher(brigid::crypto_hash::sha256);
    BRIGID_CHECK(hasher->digest() == expect);
  }

  void test_sha256_1() {
    static const char expect_data[] = {
      '\xD7', '\xA8', '\xFB', '\xB3', '\x07', '\xD7', '\x80', '\x94',
      '\x69', '\xCA', '\x9A', '\xBC', '\xB0', '\x08', '\x2E', '\x4F',
      '\x8D', '\x56', '\x51', '\xE4', '\x6D', '\x3C', '\xDB', '\x76',
      '\x2D', '\x02', '\xD0', '\xBF', '\x37', '\xC9', '\xE5', '\x92',
    };
    static const std::string expect(expect_data, 32);

    auto hasher = brigid::make_hasher(brigid::crypto_hash::sha256);
    hasher->update(plaintext.data(), plaintext.size());
    BRIGID_CHECK(hasher->digest() == expect);
  }

  void test_sha256_2() {
    static const char expect_data[] = {
      '\xD7', '\xA8', '\xFB', '\xB3', '\x07', '\xD7', '\x80', '\x94',
      '\x69', '\xCA', '\x9A', '\xBC', '\xB0', '\x08', '\x2E', '\x4F',
      '\x8D', '\x56', '\x51', '\xE4', '\x6D', '\x3C', '\xDB', '\x76',
      '\x2D', '\x02', '\xD0', '\xBF', '\x37', '\xC9', '\xE5', '\x92',
    };
    static const std::string expect(expect_data, 32);

    auto hasher = brigid::make_hasher(brigid::crypto_hash::sha256);
    for (size_t i = 0; i < plaintext.size(); ++i) {
      hasher->update(plaintext.data() + i, 1);
    }
    BRIGID_CHECK(hasher->digest() == expect);
  }

  void test_sha512_empty() {
    static const char expect_data[] = {
      '\xCF', '\x83', '\xE1', '\x35', '\x7E', '\xEF', '\xB8', '\xBD',
      '\xF1', '\x54', '\x28', '\x50', '\xD6', '\x6D', '\x80', '\x07',
      '\xD6', '\x20', '\xE4', '\x05', '\x0B', '\x57', '\x15', '\xDC',
      '\x83', '\xF4', '\xA9', '\x21', '\xD3', '\x6C', '\xE9', '\xCE',
      '\x47', '\xD0', '\xD1', '\x3C', '\x5D', '\x85', '\xF2', '\xB0',
      '\xFF', '\x83', '\x18', '\xD2', '\x87', '\x7E', '\xEC', '\x2F',
      '\x63', '\xB9', '\x31', '\xBD', '\x47', '\x41', '\x7A', '\x81',
      '\xA5', '\x38', '\x32', '\x7A', '\xF9', '\x27', '\xDA', '\x3E',
    };
    static const std::string expect(expect_data, 64);

    auto hasher = brigid::make_hasher(brigid::crypto_hash::sha512);
    BRIGID_CHECK(hasher->digest() == expect);
  }

  void test_sha512_1() {
    static const char expect_data[] = {
      '\x07', '\xE5', '\x47', '\xD9', '\x58', '\x6F', '\x6A', '\x73',
      '\xF7', '\x3F', '\xBA', '\xC0', '\x43', '\x5E', '\xD7', '\x69',
      '\x51', '\x21', '\x8F', '\xB7', '\xD0', '\xC8', '\xD7', '\x88',
      '\xA3', '\x09', '\xD7', '\x85', '\x43', '\x6B', '\xBB', '\x64',
      '\x2E', '\x93', '\xA2', '\x52', '\xA9', '\x54', '\xF2', '\x39',
      '\x12', '\x54', '\x7D', '\x1E', '\x8A', '\x3B', '\x5E', '\xD6',
      '\xE1', '\xBF', '\xD7', '\x09', '\x78', '\x21', '\x23', '\x3F',
      '\xA0', '\x53', '\x8F', '\x3D', '\xB8', '\x54', '\xFE', '\xE6',
    };
    static const std::string expect(expect_data, 64);

    auto hasher = brigid::make_hasher(brigid::crypto_hash::sha512);
    hasher->update(plaintext.data(), plaintext.size());
    BRIGID_CHECK(hasher->digest() == expect);
  }

  void test_sha512_2() {
    static const char expect_data[] = {
      '\x07', '\xE5', '\x47', '\xD9', '\x58', '\x6F', '\x6A', '\x73',
      '\xF7', '\x3F', '\xBA', '\xC0', '\x43', '\x5E', '\xD7', '\x69',
      '\x51', '\x21', '\x8F', '\xB7', '\xD0', '\xC8', '\xD7', '\x88',
      '\xA3', '\x09', '\xD7', '\x85', '\x43', '\x6B', '\xBB', '\x64',
      '\x2E', '\x93', '\xA2', '\x52', '\xA9', '\x54', '\xF2', '\x39',
      '\x12', '\x54', '\x7D', '\x1E', '\x8A', '\x3B', '\x5E', '\xD6',
      '\xE1', '\xBF', '\xD7', '\x09', '\x78', '\x21', '\x23', '\x3F',
      '\xA0', '\x53', '\x8F', '\x3D', '\xB8', '\x54', '\xFE', '\xE6',
    };
    static const std::string expect(expect_data, 64);

    auto hasher = brigid::make_hasher(brigid::crypto_hash::sha512);
    for (size_t i = 0; i < plaintext.size(); ++i) {
      hasher->update(plaintext.data() + i, 1);
    }
    BRIGID_CHECK(hasher->digest() == expect);
  }

  BRIGID_MAKE_TEST_CASE([](){ test_encryptor1(brigid::crypto_cipher::aes_128_cbc, key128, ciphertext128); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor2(brigid::crypto_cipher::aes_128_cbc, key128, ciphertext128); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor3(brigid::crypto_cipher::aes_128_cbc, key128, ciphertext128); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor1(brigid::crypto_cipher::aes_192_cbc, key192, ciphertext192); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor2(brigid::crypto_cipher::aes_192_cbc, key192, ciphertext192); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor3(brigid::crypto_cipher::aes_192_cbc, key192, ciphertext192); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor1(brigid::crypto_cipher::aes_256_cbc, key256, ciphertext256); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor2(brigid::crypto_cipher::aes_256_cbc, key256, ciphertext256); });
  BRIGID_MAKE_TEST_CASE([](){ test_encryptor3(brigid::crypto_cipher::aes_256_cbc, key256, ciphertext256); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor1(brigid::crypto_cipher::aes_128_cbc, key128, ciphertext128); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor2(brigid::crypto_cipher::aes_128_cbc, key128, ciphertext128); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor3(brigid::crypto_cipher::aes_128_cbc, key128, ciphertext128); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor1(brigid::crypto_cipher::aes_192_cbc, key192, ciphertext192); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor2(brigid::crypto_cipher::aes_192_cbc, key192, ciphertext192); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor3(brigid::crypto_cipher::aes_192_cbc, key192, ciphertext192); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor1(brigid::crypto_cipher::aes_256_cbc, key256, ciphertext256); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor2(brigid::crypto_cipher::aes_256_cbc, key256, ciphertext256); });
  BRIGID_MAKE_TEST_CASE([](){ test_decryptor3(brigid::crypto_cipher::aes_256_cbc, key256, ciphertext256); });
  BRIGID_MAKE_TEST_CASE(test_error1);
  BRIGID_MAKE_TEST_CASE(test_error2);
  BRIGID_MAKE_TEST_CASE(test_buffer_size);
  BRIGID_MAKE_TEST_CASE(test_sha256_empty);
  BRIGID_MAKE_TEST_CASE(test_sha256_1);
  BRIGID_MAKE_TEST_CASE(test_sha256_2);
  BRIGID_MAKE_TEST_CASE(test_sha512_empty);
  BRIGID_MAKE_TEST_CASE(test_sha512_1);
  BRIGID_MAKE_TEST_CASE(test_sha512_2);
}
