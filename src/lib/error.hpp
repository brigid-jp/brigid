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
  inline error_code<T> make_error_code(T code) {
    return error_code<T>("error number", code);
  }

  template <class T>
  inline error_code<T> make_error_code(const char* name, T code) {
    return error_code<T>(name, code);
  }

  class error : public std::runtime_error {
  public:
    error(const char* file, int line, const char* message)
      : std::runtime_error(make_what(file, line, message)) {}

    error(const char* file, int line, const std::string& message)
      : std::runtime_error(make_what(file, line, message.c_str())) {}

    template <class T>
    error(const char* file, int line, const error_code<T>& code)
      : std::runtime_error(make_what(file, line, code)) {}

    template <class T>
    error(const char* file, int line, const char* message, const error_code<T>& code)
      : std::runtime_error(make_what(file, line, message, code)) {}

    template <class T>
    error(const char* file, int line, const std::string& message, const error_code<T>& code)
      : std::runtime_error(make_what(file, line, message.c_str(), code)) {}

  private:
    static std::string make_what(const char* file, int line, const char* message) {
      std::ostringstream out;
      out << message << " at " << file << ":" << line;
      return out.str();
    }

    template <class T>
    static std::string make_what(const char* file, int line, const error_code<T>& code) {
      std::ostringstream out;
      out << code << " at " << file << ":" << line;
      return out.str();
    }

    template <class T>
    static std::string make_what(const char* file, int line, const char* message, const error_code<T>& code) {
      std::ostringstream out;
      out << message << " (" << code << ") at " << file << ":" << line;
      return out.str();
    }
  };
}

#define BRIGID_ERROR(...) brigid::error(__FILE__, __LINE__, __VA_ARGS__)

#endif
