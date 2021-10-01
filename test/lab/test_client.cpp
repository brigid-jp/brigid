// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>

#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

namespace brigid {
  class timer : private noncopyable {
  public:
    void start() {
      if (clock_gettime(CLOCK_MONOTONIC, &t0_) == -1) {
        throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
      }
    }

    void stop() {
      if (clock_gettime(CLOCK_MONOTONIC, &t1_) == -1) {
        throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
      }
    }

    int64_t elapsed() const {
      // t1 - t0
      struct timespec t1 = t1_;
      if (t1.tv_nsec < t0_.tv_nsec) {
        t1.tv_nsec += 1000000000;
        --t1.tv_sec;
      }
      int64_t elapsed = t1.tv_sec - t0_.tv_sec;
      elapsed *= 1000000000;
      elapsed += t1.tv_nsec - t0_.tv_nsec;
      return elapsed;
    }

    void print(const char* message) const {
      int64_t t = elapsed();
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
    struct timespec t0_;
    struct timespec t1_;
  };

  struct freeaddrinfo_t {
    void operator()(struct addrinfo* ai) {
      std::cout << "freeaddrinfo " << static_cast<void*>(ai) << "\n";
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

  void run(const char* node, const char* serv) {
    std::cout << std::setfill('0');

    timer t;

    addrinfo_t ai = getaddrinfo(node, serv, AI_ADDRCONFIG, AF_INET, SOCK_STREAM);

    int fd = socket(ai->ai_family, ai->ai_socktype, 0);
    if (fd == -1) {
      throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
    }

    if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
      throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
    }

    t.start();
    {
      std::string buffer = "GET / HTTP/1.0\r\n\r\n";
      if (write(fd, buffer.data(), buffer.size()) == -1) {
        throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
      }
    }
    t.stop();
    t.print("write");


    t.start();
    {
      std::vector<char> buffer(2048);
      while (true) {
        ssize_t size = read(fd, buffer.data(), buffer.size());
        if (size > 0) {
          std::cout << "[";
          for (ssize_t i = 0; i < size; ++i) {
            std::cout << buffer[i];
          }
          std::cout << "]\n";
        } else if (size == 0) {
          std::cout << "closed\n";
          break;
        } else {
          int code = errno;
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
        }
      }
    }
    t.stop();
    t.print("read");

    try {
      close(fd);
    } catch (...) {
      close(fd);
      throw;
    }
  }
}

int main(int ac, char* av[]) {
  try {
    if (ac < 3) {
      std::cout << "usage: " << av[0] << " node serv\n";
      return 1;
    }
    brigid::run(av[1], av[2]);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 1;
}
