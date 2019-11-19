// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"

#include <stddef.h>
#include <iostream>
#include <vector>

namespace brigid {
  namespace {
    class test_case_impl {
    public:
      test_case_impl(const std::string& name, std::function<void()> function) : name_(name), function_(function) {}

      bool operator()() const {
        try {
          function_();
          std::cout << "PASS " << name_ << "\n";
          return true;
        } catch (const std::exception& e) {
          std::cout << "FAIL " << name_ << " " << e.what() << "\n";
        } catch (...) {
          std::cout << "FAIL " << name_ << "\n";
        }
        return false;
      }

    private:
      std::string name_;
      std::function<void()> function_;
    };

    std::vector<test_case_impl> test_cases;
  }

  make_test_case::make_test_case(const std::string& name, std::function<void()> function) {
    test_cases.emplace_back(name, function);
  }

  int run_test_cases() {
    size_t pass_count = 0;
    size_t fail_count = 0;

    for (const auto& test_case : test_cases) {
      if (test_case()) {
        ++pass_count;
      } else {
        ++fail_count;
      }
    }

    if (fail_count == 0) {
      return 0;
    } else {
      return 1;
    }
  }
}
