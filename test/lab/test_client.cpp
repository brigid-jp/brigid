// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>

#include <exception>
#include <iostream>
#include <vector>

#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

namespace brigid {

  struct freeaddrinfo_t {
    void operator()(struct addrinfo* ai) {
      freeaddrinfo(ai);
    }
  };

  void run(const char* node, const char* serv) {

    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;

    struct addrinfo* result = nullptr;

    int code = getaddrinfo(node, serv, &hints, &result);
    if (code != 0) {
      throw BRIGID_RUNTIME_ERROR(gai_strerror(code), make_error_code("nedb error number", code));
    }

    std::unique_ptr<struct addrinfo, freeaddrinfo_t> ai(result);

    int fd = socket(ai->ai_family, ai->ai_socktype, 0);
    if (fd == -1) {
      int code = errno;
      throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
    }

    if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
      int code = errno;
      throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
    }

    {
      std::string buffer = "GET / HTTP/1.0\r\n\r\n";
      if (write(fd, buffer.data(), buffer.size()) == -1) {
        int code = errno;
        throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
      }
    }

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
        throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
      }
    }

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
