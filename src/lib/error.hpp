// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_ERROR_HPP
#define BRIGID_ERROR_HPP

#include <exception>
#include <sstream>
#include <string>

namespace brigid {
  class error : public std::exception {
  public:
    error(const char* file, int line, const char* message) {
      std::ostringstream out;
      out << "brigid::error " << message << " at " << file << ":" << line;
      what_ = out.str();
    }

    template <class T>
    error(const char* file, int line, const char* message, T code) {
      std::ostringstream out;
      out << "brigid::error " << message << " (error number " << code << ") at " << file << ":" << line;
      what_ = out.str();
    }

    virtual const char* what() const noexcept {
      return what_.c_str();
    }

  private:
    std::string what_;
  };
}

#define BRIGID_ERROR(...) brigid::error(__FILE__, __LINE__, __VA_ARGS__)

#endif
