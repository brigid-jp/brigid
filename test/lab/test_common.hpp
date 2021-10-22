// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_TEST_COMMON_HPP
#define BRIGID_TEST_COMMON_HPP

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>

#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <chrono>
#include <iomanip>
#include <iostream>

namespace brigid {
  class timer : private noncopyable {
  public:
    using clock_type = std::chrono::high_resolution_clock;

    void start() {
      t0_ = clock_type::now();
    }

    void stop() {
      t1_ = clock_type::now();
    }

    int64_t elapsed_nsec() const {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(t1_ - t0_).count();
    }

    void print(const char* message) const {
      int64_t t = elapsed_nsec();
      int64_t s = t / 1000000000;
      int64_t m = (t % 1000000000) / 1000000;
      int64_t u = (t % 1000000) / 1000;
      int64_t n = t % 1000;

      std::cout
        << message << ": "
        << s << "."
        << std::setw(3) << m << " "
        << std::setw(3) << u << " "
        << std::setw(3) << n << "\n";
    }

  private:
    clock_type::time_point t0_;
    clock_type::time_point t1_;
  };

  struct freeaddrinfo_t {
    void operator()(struct addrinfo* ai) {
      freeaddrinfo(ai);
    }
  };

  using addrinfo_t = std::unique_ptr<struct addrinfo, freeaddrinfo_t>;

  inline addrinfo_t getaddrinfo(const char* node, const char* serv, int flags, int family, int socktype) {
    struct addrinfo hints = {};
    hints.ai_flags = flags;
    hints.ai_family = family;
    hints.ai_socktype = socktype;

    struct addrinfo* result = nullptr;

    int code = ::getaddrinfo(node, serv, &hints, &result);
    if (code != 0) {
      throw BRIGID_RUNTIME_ERROR(gai_strerror(code), make_error_code("nedb error number", code));
    }

    return addrinfo_t(result, freeaddrinfo_t());
  }
}

#endif
