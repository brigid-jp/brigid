// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <stdint.h>
#include <stdio.h>
#include <random>

int main(int, char*[]) {
  std::independent_bits_engine<std::random_device, 8, uint8_t> engine;
  uint8_t data[32] = {};
  for (size_t i = 0; i < 32; ++i) {
    data[i] = engine();
  }
  fwrite(data, 1, 32, stdout);
  return 0;
}
