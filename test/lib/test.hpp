// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_TEST_HPP
#define BRIGID_TEST_HPP

#include <sstream>
#include <stdexcept>
#include <string>

namespace brigid {
  template <class T>
  T check_impl(T result, const char* expression, const char* file, int line, const char* function) {
    if (!result) {
      std::ostringstream out;
      out << "check failed (" << expression << ") at " << file << ":" << line << " in " << function;
      throw std::runtime_error(out.str());
    }
    return result;
  }

  struct make_test_case {
  public:
    make_test_case(const std::string&, std::function<void()>);
  };

  int run_test_cases();
}

#define BRIGID_CHECK(expression) brigid::check_impl((expression), #expression, __FILE__, __LINE__, __func__)

#endif
