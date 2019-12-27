// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

std::string encode(uint32_t value) {
  std::ostringstream out;
  out << "0x"
      << std::hex
      << std::uppercase
      << std::setfill('0')
      << std::setw(8)
      << value;
  return out.str();
}

int main(int, char*[]) {
  std::independent_bits_engine<std::random_device, 8, uint8_t> engine;




  std::string data[] = {
    encode(engine()),
    encode(engine()),
    encode(engine()),
    encode(engine()),
    encode(engine()),
    encode(engine()),
    encode(engine()),
    encode(engine()),
  };

  std::ostream& out = std::cout;

  out << "/*\n";
  for (int i = 0; i < 8; ++i) {
    out << "BRIGID_ZONE" << i + 1 << "=" << data[i] << "\n";
  }
  out << "export";
  for (int i = 0; i < 8; ++i) {
    out << " BRIGID_ZONE" << i + 1;
  }
  out << "\n";
  out << "*/\n";

  for (int i = 0; i < 8; ++i) {
    out << "#if !(BRIGID_ZONE" << i + 1 << "+0)\n"
        << "#define BRIGID_ZONE" << i + 1 << " " << data[i] << "\n"
        << "#endif\n";
  }

  // for (int i = 0; i < 8; ++i) {
  //   printf("export BRIGID_ZONE%d=0x%08X\n", i + 1, data[i]);
  // }

  // for (int i = 0; i < 8; ++i) {
  //   printf("set BRIGID_ZONE%d=0x%08X\n", i + 1, data[i]);
  // }

  return 0;
}
