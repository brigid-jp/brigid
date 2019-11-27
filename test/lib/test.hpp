// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_TEST_HPP
#define BRIGID_TEST_HPP

#include <functional>
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

  template <class T>
  void check_throw_impl(T fn, const char* expression, const char* file, int line, const char* function) {
    try {
      fn();
    } catch (...) {
      return;
    }
    std::ostringstream out;
    out << "check throw failed (" << expression << ") at " << file << ":" << line << " in " << function;
    throw std::runtime_error(out.str());
  }

  struct make_test_case {
  public:
    make_test_case(const std::string&, std::function<void ()>);
    make_test_case(const std::string&, const std::string&, std::function<void ()>);
  };

  int run_test_cases();
}

#ifdef _MSC_VER
#define BRIGID_CHECK(expression) brigid::check_impl((expression), #expression, __FILE__, __LINE__, __FUNCTION__)
#define BRIGID_CHECK_THROW(expression) brigid::check_throw_impl((expression), #expression, __FILE__, __LINE__, __FUNCTION__)
#else
#define BRIGID_CHECK(expression) brigid::check_impl((expression), #expression, __FILE__, __LINE__, __func__)
#define BRIGID_CHECK_THROW(expression) brigid::check_throw_impl((expression), #expression, __FILE__, __LINE__, __func__)
#endif

#define BRIGID_MAKE_TEST_CASE_NAME_IMPL(a, b) a ## b
#define BRIGID_MAKE_TEST_CASE_NAME(a, b) BRIGID_MAKE_TEST_CASE_NAME_IMPL(a, b)
#define BRIGID_MAKE_TEST_CASE(expression) brigid::make_test_case BRIGID_MAKE_TEST_CASE_NAME(brigid_make_test_case_, __LINE__)(__FILE__, #expression, (expression))

#endif
