// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "http_request_parser.hpp"

#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace brigid {
  namespace {
    struct freeaddrinfo_t {
      void operator()(struct addrinfo* ai) {
        freeaddrinfo(ai);
      }
    };

    using addrinfo_t = std::unique_ptr<struct addrinfo, freeaddrinfo_t>;

    enum class connection_state {
      opening,
      opened,
      closed,
      error,
    };

    const char* base64_encoder =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

    inline std::string base64_encode(const std::string& source) {
      std::string result;

      size_t n = source.size();

      for (size_t i = 2; i < n; i += 3) {
        uint8_t x = source[i - 2];
        uint8_t y = source[i - 1];
        uint8_t z = source[i];
        uint32_t a = x << 16 | y << 8 | z;
        uint32_t d = a & 0x3F; a >>= 6;
        uint32_t c = a & 0x3F; a >>= 6;
        uint32_t b = a & 0x3F; a >>= 6;
        result += base64_encoder[a];
        result += base64_encoder[b];
        result += base64_encoder[c];
        result += base64_encoder[d];
      }

      switch (n % 3) {
        case 1:
          {
            uint8_t x = source[n - 1];
            uint32_t a = x << 4;
            uint32_t b = a & 0x3F; a >>= 6;
            result += base64_encoder[a];
            result += base64_encoder[b];
            result += "==";
          }
          break;
        case 2:
          {
            uint8_t x = source[n - 2];
            uint8_t y = source[n - 1];
            uint32_t a = x << 10 | y << 2;
            uint32_t b = a & 0x3F; a >>= 6;
            uint32_t c = a & 0x3F; a >>= 6;
            result += base64_encoder[a];
            result += base64_encoder[b];
            result += base64_encoder[c];
            result += '=';
          }
          break;
      }

      return result;
    }

    class connection {
    public:
      explicit connection(int fd) : fd_(fd), state_(connection_state::opening) {}

      void read(std::map<int, connection>& connections, const char* data, size_t size) {
        switch (state_) {
          case connection_state::opening:
            {
              auto result = parser_.parse(data, size);
              switch (result.first) {
                case parser_state::running:
                  break;
                case parser_state::accept:
                  for (size_t i = 0; ; ++i) {
                    const auto& header_field = parser_.header_field(i);
                    if (!header_field.first) {
                      break;
                    }
                    header_fields_.emplace(header_field.first, header_field.second);
                  }





                  state_ = connection_state::opened;
                  return read(connections, result.second, size - (result.second - data));
                case parser_state::error:
                  // send error response
                  state_ = connection_state::error;
                  break;
              }
            }
            break;
          case connection_state::opened:
            break;
          case connection_state::error:
            break;
        }
      }

    private:
      int fd_;
      connection_state state_;
      http_request_parser parser_;
      std::map<std::string, std::string> header_fields_;
    };

    void run(const char* node, const char* serv) {
      int server_fd = -1;
      try {
        {
          struct sigaction sa = {};
          sa.sa_handler = SIG_IGN;
          if (sigaction(SIGPIPE, &sa, 0) == -1) {
            throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
          }
        }

        addrinfo_t ai;
        {
          struct addrinfo hints = {};
          hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
          hints.ai_family = AF_UNSPEC;
          hints.ai_socktype = SOCK_STREAM;
          struct addrinfo* result = nullptr;
          int code = getaddrinfo(node, serv, &hints, &result);
          if (code != 0) {
            throw BRIGID_RUNTIME_ERROR(gai_strerror(code), make_error_code("nedb error number", code));
          }
          ai = addrinfo_t(result, freeaddrinfo_t());
        }

        server_fd = socket(ai->ai_family, ai->ai_socktype, 0);
        if (server_fd == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }
        if (bind(server_fd, ai->ai_addr, ai->ai_addrlen) == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }
        if (listen(server_fd, SOMAXCONN) == -1) {
          throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
        }

        {
          int result = fcntl(server_fd, F_GETFL);
          if (result == -1) {
            throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
          }
          if (fcntl(server_fd, F_SETFL, result | O_NONBLOCK) == -1) {
            throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
          }
        }

        {
          int v = 1;
          if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v)) == -1) {
            throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
          }
        }

        std::map<int, connection> connections;
        std::vector<char> buffer(4096);

        while (true) {
          int fd_max = 0;
          fd_set rfds; FD_ZERO(&rfds);
          fd_set wfds; FD_ZERO(&wfds);
          fd_set efds; FD_ZERO(&efds);
          struct timeval timeout = {};
          timeout.tv_sec = 1;

          FD_SET(server_fd, &rfds);
          for (const auto& item : connections) {
            FD_SET(item.first, &rfds);
          }

          int result = select(fd_max, &rfds, &wfds, &efds, &timeout);
          std::cout << result << "\n";
          if (result == -1) {
            if (errno == EINTR) {
              continue;
            }
            throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
          }

          if (FD_ISSET(server_fd, &rfds)) {
            while (true) {
              int fd = accept(server_fd, nullptr, nullptr);
              if (fd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                  break;
                }
                throw BRIGID_RUNTIME_ERROR(std::generic_category().message(errno), make_error_code("error number", errno));
              }
              connections.emplace(fd, connection(fd));
            }
          }

          for (auto& item : connections) {
            if (FD_ISSET(item.first, &rfds)) {
              while (true) {
                ssize_t size = read(item.first, buffer.data(), buffer.size());
                item.second.read(connections, buffer.data(), size);
              }
            }
          }
        }

      } catch (...) {
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
    std::cout << brigid::base64_encode(av[1]) << "\n";

    if (ac < 3) {
      std::cout << "usage: " << av[0] << " node serv\n";
    }
    // brigid::run(av[1], av[2]);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 1;
}
