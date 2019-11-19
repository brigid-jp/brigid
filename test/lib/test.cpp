// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"

#include <iostream>
#include <vector>

namespace brigid {
  namespace {
    struct test_case_impl {
      std::string name;
      std::function<void()> function;
    };

    std::vector<test_case_impl> test_suite;
  }

  test_case::test_case(const std::string& name, std::function<void()> function) {
    test_suite.push_back({ name, function });
  }

  int run_test_suite() {
    int result = 0;
    for (const auto& test_case : test_suite) {
      try {
        test_case.function();
        std::cout << "PASS " << test_case.name << "\n";
      } catch (const std::exception& e) {
        result = 1;
        std::cout << "FAIL " << test_case.name << " " << e.what() << "\n";
      }
    }
    return result;
  }
}
