// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/noncopyable.hpp>
#include "error.hpp"
#include "http_impl.hpp"

#include <stdio.h>
#include <string.h>
#include <algorithm>

namespace brigid {
  namespace {
    FILE* check(FILE* handle) {
      if (!handle) {
        throw BRIGID_ERROR("cannot fopen", errno);
      }
      return handle;
    }

    using file_t = std::unique_ptr<FILE, decltype(&fclose)>;

    file_t make_file(FILE* handle) {
      return file_t(handle, &fclose);
    }

    class http_reader_impl : public http_reader {
    public:
      http_reader_impl()
        : total_(),
          now_() {}

      virtual size_t total() const {
        return total_;
      }

      virtual size_t now() const {
        return now_;
      }

    protected:
      void set_total(size_t total) {
        total_ = total;
      }

      void add_now(size_t now) {
        now_ += now;
      }

    private:
      size_t total_;
      size_t now_;
    };

    class http_data_reader : public http_reader_impl, private noncopyable {
    public:
      http_data_reader(const char* data, size_t size)
        : data_(data),
          size_(size) {
        set_total(size);
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = std::min(size, size_);
        memmove(data, data_, result);
        data_ += result;
        size_ -= result;
        add_now(result);
        return result;
      }

    private:
      const char* data_;
      size_t size_;
    };

    class http_file_reader : public http_reader_impl, private noncopyable {
    public:
      http_file_reader(const char* path)
        : handle_(make_file(check(fopen(path, "rb")))) {
        fseek(handle_.get(), 0, SEEK_END);
        set_total(ftell(handle_.get()));
        fseek(handle_.get(), 0, SEEK_SET);
      }

      virtual size_t read(char* data, size_t size) {
        size_t result = fread(data, 1, size, handle_.get());
        add_now(result);
        return result;
      }

    private:
      file_t handle_;
    };

  }

  std::unique_ptr<http_reader> make_http_reader(http_request_body body, const char* data, size_t size) {
    switch (body) {
      case http_request_body::data:
        if (data) {
          return std::unique_ptr<http_reader>(new http_data_reader(data, size));
        }
        break;
      case http_request_body::file:
        return std::unique_ptr<http_reader>(new http_file_reader(data));
    }
    return std::unique_ptr<http_reader>();
  }
}
