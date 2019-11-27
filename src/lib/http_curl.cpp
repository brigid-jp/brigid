// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>

#include <curl/curl.h>

#include <stdio.h>
#include <iostream>
#include <vector>

namespace brigid {
  namespace {
    void check(CURLcode code) {
      if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
      }
    }

    CURL* check(CURL* result) {
      if (!result) {
        check(CURLE_FAILED_INIT);
      }
      return result;
    }

    using easy_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

    easy_t make_easy(CURL* easy) {
      return easy_t(easy, &curl_easy_cleanup);
    }

    class string_list {
    public:
      string_list()
        : slist_() {}

      ~string_list() {
        curl_slist_free_all(slist_);
      }

      void append(const std::string& value) {
        if (curl_slist* slist = curl_slist_append(slist_, value.c_str())) {
          slist_ = slist;
        } else {
          throw std::runtime_error("slist error");
        }
      }

      curl_slist* get() const {
        return slist_;
      }

    private:
      curl_slist* slist_;
    };

    class http_session_impl : public http_session {
    public:
      http_session_impl()
        : handle_(make_easy(check(curl_easy_init()))),
          credential_(),
          file_(nullptr) {}

      virtual void set_progress_cb(std::function<bool (size_t, size_t)> progress_cb) {
        progress_cb_ = progress_cb;
      }

      virtual void set_header_cb(std::function<bool (int, const std::map<std::string, std::string>&)> header_cb) {
        header_cb_ = header_cb;
      }

      virtual void set_write_cb(std::function<bool (const char*, size_t)> write_cb) {
        write_cb_ = write_cb;
      }

      virtual void set_credential() {
        credential_ = false;
        username_.clear();
        password_.clear();
      }

      virtual void set_credential(const std::string& username, const std::string& password) {
        credential_ = true;
        username_ = username;
        password_ = password;
      }

      virtual void request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& headers,
          http_request_body body,
          const char* data,
          size_t size) {
        curl_easy_reset(handle_.get());
        check(curl_easy_setopt(handle_.get(), CURLOPT_VERBOSE, 1));
        check(curl_easy_setopt(handle_.get(), CURLOPT_CUSTOMREQUEST, method.c_str()));
        check(curl_easy_setopt(handle_.get(), CURLOPT_URL, url.c_str()));

        string_list headers_list;
        if (!headers.empty()) {
          for (const auto& header : headers) {
            headers_list.append(header.first + ": " + header.second);
          }
          check(curl_easy_setopt(handle_.get(), CURLOPT_HTTPHEADER, headers_list.get()));
        }

        switch (body) {
          case http_request_body::data:
            if (data) {
              check(curl_easy_setopt(handle_.get(), CURLOPT_POSTFIELDS, data));
              check(curl_easy_setopt(handle_.get(), CURLOPT_POSTFIELDSIZE_LARGE, size));
            }
            break;
          case http_request_body::file:
            if (file_) {
              fclose(file_);
              file_ = nullptr;

            }
            file_ = fopen(data, "rb");
            if (!file_) {
              throw std::runtime_error("could not open");
            }
            fseek(file_, 0, SEEK_END);
            progress_now_ = 0;
            progress_total_ = ftell(file_);
            check(curl_easy_setopt(handle_.get(), CURLOPT_UPLOAD, 1));
            check(curl_easy_setopt(handle_.get(), CURLOPT_INFILESIZE_LARGE, progress_total_));
            check(curl_easy_setopt(handle_.get(), CURLOPT_READFUNCTION, &http_session_impl::read_cb));
            check(curl_easy_setopt(handle_.get(), CURLOPT_READDATA, this));
            fseek(file_, 0, SEEK_SET);
            break;
        }

        if (header_cb_) {
          header_data_.clear();
          check(curl_easy_setopt(handle_.get(), CURLOPT_HEADERFUNCTION, &http_session_impl::header_cb));
          check(curl_easy_setopt(handle_.get(), CURLOPT_HEADERDATA, this));
        }
        if (write_cb_) {
          check(curl_easy_setopt(handle_.get(), CURLOPT_WRITEFUNCTION, &http_session_impl::write_cb));
          check(curl_easy_setopt(handle_.get(), CURLOPT_WRITEDATA, this));
        }

        curl_easy_setopt(handle_.get(), CURLOPT_FOLLOWLOCATION, 1);

        if (credential_) {
          std::cerr << "credential " << username_ << " " << password_ << "\n";
          curl_easy_setopt(handle_.get(), CURLOPT_HTTPAUTH, CURLAUTH_ANY);
          curl_easy_setopt(handle_.get(), CURLOPT_USERNAME, username_.c_str());
          curl_easy_setopt(handle_.get(), CURLOPT_PASSWORD, password_.c_str());
        }

        CURLcode code = curl_easy_perform(handle_.get());

        if (file_) {
          fclose(file_);
          file_ = nullptr;
        }

        if (exception_) {
          std::exception_ptr exception = exception_;
          exception_ = nullptr;
          std::rethrow_exception(exception);
        }
        check(code);

        curl_easy_reset(handle_.get());
      }

    private:
      easy_t handle_;
      std::function<bool (size_t, size_t)> progress_cb_;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
      bool credential_;
      std::string username_;
      std::string password_;
      FILE* file_;
      size_t progress_now_;
      size_t progress_total_;
      std::vector<std::string> header_data_;
      std::exception_ptr exception_;

      static size_t read_cb(char* data, size_t m, size_t n, void* self) {
        return static_cast<http_session_impl*>(self)->read(data, m, n);
      }

      static size_t header_cb(char* data, size_t m, size_t n, void* self) {
        return static_cast<http_session_impl*>(self)->header(data, m * n);
      }

      static size_t write_cb(char* data, size_t m, size_t n, void* self) {
        return static_cast<http_session_impl*>(self)->write(data, m * n);
      }

      size_t read(char* data, size_t m, size_t n) {
        if (file_) {
          size_t result = fread(data, m, n, file_);
          try {
            if (progress_cb_) {
              progress_now_ += result;
              if (progress_cb_(progress_now_, progress_total_)) {
                return result;
              }
            }
          } catch (...) {
            if (!exception_) {
              exception_ = std::current_exception();
            }
          }
        }
        return CURL_READFUNC_ABORT;
      }

      size_t header(const char* data, size_t size) {
        try {
          // should parse strictly
          {
            std::string line(data, size);
            // std::cout << "[" << line << "]";

            if (line != "\r\n") {
              size_t n = line.size();
              if (n > 2) {
                line.resize(n - 2);
              }
              size_t i = 0;
              for (; i < line.size(); ++i) {
                if (line[i] != ' ' && line[i] != '\t') {
                  break;
                }
              }
              if (i > 0) {
                line.resize(i);
                header_data_.back() += " " + line;
              } else {
                header_data_.push_back(line);
              }
              return size;
            }
          }

          std::map<std::string, std::string> headers;

          for (size_t i = 1; i < header_data_.size(); ++i) {
            std::string header = header_data_[i];
            size_t p = 0;
            for (size_t i = 0; i < header.size(); ++i) {
              if (header[i] == ':') {
                p = i;
                break;
              }
            }
            if (p > 0) {
              size_t q = p + 1;
              for (; q < header.size(); ++q) {
                if (header[q] != ' ') {
                  break;
                }
              }

              std::string key = header.substr(0, p);
              std::string value = header.substr(q);
              // std::cout << "[" << key << "]={" << value << "}\n";
              headers[key] = value;
            }
          }

          long code = 0;
          curl_easy_getinfo(handle_.get(), CURLINFO_RESPONSE_CODE, &code);
          // std::cout << "CODE " << code << "\n";

          if (header_cb_(code, headers)) {
            return size;
          }
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return 0;
      }

      size_t write(const char* data, size_t size) {
        try {
          if (write_cb_(data, size)) {
            return size;
          }
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return 0;
      }
    };
  }

  std::unique_ptr<http_session> make_http_session() {
    return std::unique_ptr<http_session>(new http_session_impl());
  }
}
