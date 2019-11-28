// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_ERROR_HPP
#define BRIGID_ERROR_HPP

#include <sstream>
#include <stdexcept>
#include <string>

namespace brigid {
  class error : public std::runtime_error {
  public:
    error(const char* file, int line, const char* message)
      : std::runtime_error(make_what3(file, line, message)) {}

    template <class T>
    error(const char* file, int line, T code)
      : std::runtime_error(make_what(file, line, nullptr, code)) {}

    template <class T>
    error(const char* file, int line, const char* message, T code)
      : std::runtime_error(make_what(file, line, message, code)) {}

    template <class T>
    error(const char* file, int line, const std::string& message, T code)
      : std::runtime_error(make_what(file, line, message.c_str(), code)) {}

  private:
    static std::string make_what3(const char* file, int line, const char* message) {
      std::ostringstream out;
      out << message << " at " << file << ":" << line;
      return out.str();
    }

    template <class T>
    static std::string make_what(const char* file, int line, const char* message, T code) {
      std::ostringstream out;
      if (message) {
        out << message << " (error number " << code << ") at " << file << ":" << line;
      } else {
        out << "error number " << code << " at " << file << ":" << line;
      }
      return out.str();
    }
  };
}

#define BRIGID_ERROR(...) brigid::error(__FILE__, __LINE__, __VA_ARGS__)

#endif
