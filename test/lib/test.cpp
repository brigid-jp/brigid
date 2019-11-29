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
      test_case_impl(const std::string& file, const std::string& name, std::function<void ()> function)
        : file_(file),
          name_(name),
          function_(function) {}

      bool operator()() const {
        try {
          function_();
          std::cout << "PASS " << file_ << ":" << name_ << "\n";
          return true;
        } catch (const std::exception& e) {
          std::cout << "FAIL " << file_ << ":" << name_ << " " << e.what() << "\n";
        } catch (...) {
          std::cout << "FAIL " << file_ << ":" << name_ << "\n";
        }
        return false;
      }

    private:
      std::string file_;
      std::string name_;
      std::function<void ()> function_;
    };

    std::vector<test_case_impl> test_cases;
  }

  make_test_case::make_test_case(const std::string& file, const std::string& name, std::function<void ()> function) {
    test_cases.emplace_back(file, name, function);
  }

  int run_test_cases() {
    size_t pass = 0;
    size_t fail = 0;

    for (const auto& test_case : test_cases) {
      if (test_case()) {
        ++pass;
      } else {
        ++fail;
      }
    }
    std::cout
        << "============================================================\n"
        << "TOTAL: " << test_cases.size() << "\n"
        << "PASS:  " << pass << "\n"
        << "FAIL:  " << fail << "\n"
        << "============================================================\n";

    if (fail == 0) {
      return 0;
    } else {
      return 1;
    }
  }
}
