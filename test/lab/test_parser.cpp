// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/stdio.hpp>
#include "http_request_parser.hpp"
#include "test_common.hpp"

#include <exception>
#include <iostream>
#include <vector>

#include <errno.h>
#include <stddef.h>

namespace brigid {
  namespace {
    void test(int ac, char* av[]) {
      if (ac < 3) {
        throw std::runtime_error("invalid arguments");
      }

      const char* path = av[2];
      http_request_parser parser;

      std::vector<char> buffer(4096);
      file_handle_t handle = open_file_handle(path, "rb");

      while (true) {
        size_t result = fread(buffer.data(), 1, buffer.size(), handle.get());
        if (result == 0) {
          break;
        }

        const auto p = parser.parse(buffer.data(), result);
        if (p.first == parser_state::accept) {
          std::cout
            << "accept position " << parser.position() << "\n"
            << "method " << parser.method() << "\n"
            << "request_target " << parser.request_target() << "\n"
            << "http_version " << parser.http_version() << "\n";
          for (const auto& header_field : parser.header_fields()) {
            std::cout << header_field.first << ": " << header_field.second << "\n";
          }
          std::cout << "[" << p.second << "]\n";
          break;
        } else if (p.first == parser_state::error) {
          std::cout << "error position " << parser.position() << "\n";
          std::cout << "[" << p.second << "]\n";
          break;
        } else {
          std::cout << "running\n";
        }
      }
    }

    void bench(int ac, char* av[]) {
      int n = 1000;

      if (ac < 3) {
        throw std::runtime_error("invalid arguments");
      }
      const char* path = av[2];
      if (ac >= 4) {
        n = atoi(av[3]);
      }

      std::vector<char> buffer(4096);
      file_handle_t handle = open_file_handle(path, "rb");
      size_t result = fread(buffer.data(), 1, buffer.size(), handle.get());
      std::cout << "size " << result << "\n";

      timer t;
      http_request_parser parser;

      t.start();
      for (int i = 0; i < n; ++i) {
        parser.clear();
        parser.parse(buffer.data(), result);
      }
      t.stop();
      t.print("parse");
    }
  }
}

int main(int ac, char* av[]) {
  try {
    if (ac < 2) {
      std::cout << "usage: " << av[0] << " command ...\n";
      return 1;
    }
    std::string command = av[1];
    if (command == "test") {
      brigid::test(ac, av);
    } else if (command == "bench") {
      brigid::bench(ac, av);
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 1;
}
