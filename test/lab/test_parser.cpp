// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/stdio.hpp>
#include "http_request_parser.hpp"

#include <exception>
#include <iostream>
#include <vector>

#include <errno.h>
#include <stddef.h>

namespace brigid {
  namespace {
    void run(const char* path) {
      http_request_parser parser;

      std::vector<char> buffer(1);
      file_handle_t handle = open_file_handle(path, "rb");

      while (true) {
        size_t result = fread(buffer.data(), 1, buffer.size(), handle.get());
        if (result == 0) {
          break;
        }

        const auto p = parser.parse(buffer.data(), result);
        if (p.first == parser_state::accept) {
          std::cout
            << "accept position " << parser.position()
            << " [" << parser.line() << ":" << parser.column() << "]\n"
            << "method " << parser.method() << "\n"
            << "request_target " << parser.request_target() << "\n"
            << "http_version " << parser.http_version() << "\n";
          for (const auto& header_field : parser.header_fields()) {
            std::cout << header_field.first << ": " << header_field.second << "\n";
          }
          break;
        } else if (p.first == parser_state::error) {
          std::cout
            << "error position " << parser.position()
            << " [" << parser.line() << ":" << parser.column() << "]\n";
          break;
        } else {
          std::cout << "running\n";
        }
      }
    }
  }
}

int main(int ac, char* av[]) {
  try {
    if (ac < 2) {
      std::cout << "usage: " << av[0] << " file\n";
      return 1;
    }
    brigid::run(av[1]);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 1;
}
