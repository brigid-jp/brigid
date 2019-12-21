// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_ERROR_HPP
#define BRIGID_ERROR_HPP

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace brigid {
  template <class T>
  class error_code {
  public:
    error_code(const char* name, T code)
      : name_(name),
        code_(code) {}

    friend std::ostream& operator<<(std::ostream& out, const error_code& self) {
      return out << self.name_ << " " << self.code_;
    }

  private:
    const char* name_;
    T code_;
  };

  template <class T>
  inline error_code<T> make_error_code(const char* name, T code) {
    return error_code<T>(name, code);
  }

  std::string make_error_impl(const char*, int, const char*);

  template <class T>
  inline std::string make_error_impl(const char* file, int line, const error_code<T>& code) {
    std::ostringstream out;
    out << code << " at " << file << ":" << line;
    return out.str();
  }

  template <class T>
  inline std::string make_error_impl(const char* file, int line, const char* message, const error_code<T>& code) {
    std::ostringstream out;
    out << message << " (" << code << ") at " << file << ":" << line;
    return out.str();
  }

  template <class T>
  class error : public T {
  public:
    error(const char* file, int line, const char* message)
      : T(make_error_impl(file, line, message)) {}

    error(const char* file, int line, const std::string& message)
      : T(make_error_impl(file, line, message.c_str())) {}

    template <class U>
    error(const char* file, int line, const error_code<U>& code)
      : T(make_error_impl(file, line, code)) {}

    template <class U>
    error(const char* file, int line, const char* message, const error_code<U>& code)
      : T(make_error_impl(file, line, message, code)) {}

    template <class U>
    error(const char* file, int line, const std::string& message, const error_code<U>& code)
      : T(make_error_impl(file, line, message.c_str(), code)) {}
  };
}

#define BRIGID_LOGIC_ERROR(...) brigid::error<std::logic_error>(__FILE__, __LINE__, __VA_ARGS__)
#define BRIGID_RUNTIME_ERROR(...) brigid::error<std::runtime_error>(__FILE__, __LINE__, __VA_ARGS__)

#endif
