// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "test_common.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

namespace brigid {
  namespace {
    void run(const char* node, const char* serv) {
      int server_fd = -1;
      int fd = -1;
      try {
        std::cout << std::setfill('0');
        timer t;

        t.start();
        addrinfo_t ai = getaddrinfo(node, serv, AI_ADDRCONFIG | AI_PASSIVE, AF_INET, SOCK_STREAM);
        t.stop();
        t.print("getaddrinfo");

        t.start();
        server_fd = socket(ai->ai_family, ai->ai_socktype, 0);
        if (server_fd == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }
        t.stop();
        t.print("socket");

        t.start();
        if (bind(server_fd, ai->ai_addr, ai->ai_addrlen) == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }
        t.stop();
        t.print("bind");

        t.start();
        if (listen(server_fd, SOMAXCONN) == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }
        t.stop();
        t.print("listen");

        t.start();
        fd = accept(server_fd, nullptr, nullptr);
        if (fd == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }
        t.stop();
        t.print("accept");

        t.start();
        {
          std::vector<char> buffer(2048);
          ssize_t total = 0;
          while (true) {
            ssize_t size = read(fd, buffer.data(), buffer.size());
            if (size > 0) {
              // std::cout << "[";
              // for (ssize_t i = 0; i < size; ++i) {
              //   std::cout << buffer[i];
              // }
              // std::cout << "]\n";
              total += size;
            } else if (size == 0) {
              std::cout << "closed " << total << "\n";
              break;
            } else {
              int code = errno;
              throw BRIGID_RUNTIME_ERROR(std::generic_category().message(code), make_error_code("error number", code));
            }
          }
        }
        t.stop();
        t.print("read");

        t.start();
        {
          std::string buffer = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
          if (write(fd, buffer.data(), buffer.size()) == -1) {
            throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
          }
        }
        t.stop();
        t.print("write");

        close(fd);
        close(server_fd);
      } catch (...) {
        if (fd != -1) {
          close(fd);
        }
        if (server_fd != -1) {
          close(server_fd);
        }
        throw;
      }
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
