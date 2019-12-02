// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "test.hpp"

#include <stddef.h>
#include <exception>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
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
          std::cout << "PASS " << *this << "\n";
          return true;
        } catch (const std::exception& e) {
          std::cout << "FAIL " << *this << " " << e.what() << "\n";
        } catch (...) {
          std::cout << "FAIL " << *this << "\n";
        }
        return false;
      }

      friend std::ostream& operator<<(std::ostream& out, const test_case_impl& self) {
        return out << self.file_ << ":" << self.name_;
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

  int run_test_cases(int ac, char* av[]) {
    try {
      std::string command = "run";
      bool run_all = true;
      size_t run_each = 0;

      if (av) {
        if (ac > 1) {
          command = av[1];
        }
        if (command == "run" && ac > 2) {
          run_all = false;
          run_each = std::stoull(av[2]);
        }
      }

      if (command == "run") {
        size_t total = 0;
        size_t pass = 0;
        size_t fail = 0;

        for (size_t i = 0; i < test_cases.size(); ++i) {
          if (run_all || run_each == i) {
            ++total;
            if (test_cases[i]()) {
              ++pass;
            } else {
              ++fail;
            }
          }
        }
        std::cout
            << "============================================================\n"
            << "TOTAL: " << total << "\n"
            << "PASS:  " << pass << "\n"
            << "FAIL:  " << fail << "\n"
            << "============================================================\n";

        if (fail == 0) {
          return 0;
        } else {
          return 1;
        }
      } else if (command == "list") {
        for (size_t i = 0; i < test_cases.size(); ++i) {
          std::cout << "[" << i << "] " << test_cases[i] << "\n";
        }
        return 0;
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
    }
    return 1;
  }
}
