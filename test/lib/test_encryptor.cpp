#include <assert.h>

#include <iostream>
#include <string>
#include <vector>

#include <brigid/crypto.hpp>

int main(int, char*[]) {
  std::string data { "The quick brown fox jumps over the lazy dog" };
  std::string key { "01234567890123456789012345678901" };
  std::string iv { "01234567890123456789012345678901" };

  std::vector<char> buffer(data.size() + 16);

  brigid::encryptor enc { "aes-256-cbc", key.data(), key.size(), iv.data(), iv.size() };
  size_t result = enc.update(data.data(), data.size(), buffer.data(), buffer.size(), true);
  buffer.resize(result);

  static const int expect[] = {
    0xE0, 0x6F, 0x63, 0xA7, 0x11, 0xE8, 0xB7, 0xAA, 0x9F, 0x94, 0x40, 0x10, 0x7D, 0x46, 0x80, 0xA1,
    0x17, 0x99, 0x43, 0x80, 0xEA, 0x31, 0xD2, 0xA2, 0x99, 0xB9, 0x53, 0x02, 0xD4, 0x39, 0xB9, 0x70,
    0x2C, 0x8E, 0x65, 0xA9, 0x92, 0x36, 0xEC, 0x92, 0x07, 0x04, 0x91, 0x5C, 0xF1, 0xA9, 0x8A, 0x44,
  };

  assert(result == sizeof(expect) / sizeof(int));

  for (size_t i = 0; i < result; ++i) {
    assert(static_cast<uint8_t>(buffer[i]) == expect[i]);
  }

  return 0;
}
