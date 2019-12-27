// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <stdint.h>
#include <stdio.h>
#include <random>

int main(int, char*[]) {
  std::independent_bits_engine<std::random_device, 32, uint32_t> engine;

  uint32_t data[] = {
    engine(),
    engine(),
    engine(),
    engine(),
    engine(),
    engine(),
    engine(),
    engine(),
  };

  for (int i = 0; i < 8; ++i) {
    printf("#define BRIGID_ZONE%d 0x%08X\n", i + 1, data[i]);
  }

  for (int i = 0; i < 8; ++i) {
    printf("export BRIGID_ZONE%d=0x%08X\n", i + 1, data[i]);
  }

  for (int i = 0; i < 8; ++i) {
    printf("set BRIGID_ZONE%d=0x%08X\n", i + 1, data[i]);
  }

  return 0;
}
