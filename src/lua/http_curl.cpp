// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "error.hpp"
#include "http.hpp"
#include "http_impl.hpp"
#include "noncopyable.hpp"

#include <curl/curl.h>

#include <stddef.h>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace brigid {
  namespace {
    void check(CURLcode code) {
      if (code != CURLE_OK) {
        throw BRIGID_RUNTIME_ERROR(curl_easy_strerror(code), make_error_code("curl error", code));
      }
    }

    CURL* check(CURL* handle) {
      if (!handle) {
        check(CURLE_FAILED_INIT);
      }
      return handle;
    }

    using easy_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

    easy_t make_easy(CURL* handle) {
      return easy_t(handle, &curl_easy_cleanup);
    }

    class http_session_impl : public http_session, private noncopyable {
    public:
      http_session_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : handle(make_easy(check(curl_easy_init()))),
          progress_cb(progress_cb),
          header_cb(header_cb),
          write_cb(write_cb),
          credential(credential),
          username(username),
          password(password) {}

      virtual bool request(const std::string&, const std::string&, const std::map<std::string, std::string>&, http_request_body, const char*, size_t);

      easy_t handle;
      std::function<bool (size_t, size_t)> progress_cb;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb;
      std::function<bool (const char*, size_t)> write_cb;
      bool credential;
      std::string username;
      std::string password;
    };

    class string_list : private noncopyable {
    public:
      string_list()
        : slist_() {}

      ~string_list() {
        curl_slist_free_all(slist_);
      }

      void append(const std::string& string) {
        if (curl_slist* slist = curl_slist_append(slist_, string.c_str())) {
          slist_ = slist;
        } else {
          throw BRIGID_RUNTIME_ERROR("cannot curl_slist_append");
        }
      }

      curl_slist* get() const {
        return slist_;
      }

    private:
      curl_slist* slist_;
    };

    class http_task : private noncopyable {
    public:
      http_task(http_session_impl& session)
        : session_(session),
          code_(-1),
          canceling_() {}

      ~http_task() {
        curl_easy_reset(session_.handle.get());
      }

      bool request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& header,
          http_request_body body,
          const char* data,
          size_t size) {
        setopt(CURLOPT_FOLLOWLOCATION, 1);
        setopt(CURLOPT_MAXREDIRS, 19);

        setopt(CURLOPT_CUSTOMREQUEST, method.c_str());
        if (method == "HEAD") {
          setopt(CURLOPT_NOBODY, 1);
        }
        setopt(CURLOPT_URL, url.c_str());

        for (const auto& field : header) {
          header_.append(field.first + ": " + field.second);
        }
        setopt(CURLOPT_HTTPHEADER, header_.get());

        if (reader_ = make_http_reader(body, data, size)) {
          setopt(CURLOPT_UPLOAD, 1);
          setopt(CURLOPT_INFILESIZE_LARGE, reader_->total());
          setopt(CURLOPT_READFUNCTION, &http_task::read_cb);
          setopt(CURLOPT_READDATA, this);
        }

        setopt(CURLOPT_HEADERFUNCTION, &http_task::header_cb);
        setopt(CURLOPT_HEADERDATA, this);

        setopt(CURLOPT_WRITEFUNCTION, &http_task::write_cb);
        setopt(CURLOPT_WRITEDATA, this);

        if (session_.credential) {
          setopt(CURLOPT_HTTPAUTH, CURLAUTH_ANY);
          setopt(CURLOPT_USERNAME, session_.username.c_str());
          setopt(CURLOPT_PASSWORD, session_.password.c_str());
        }

        CURLcode code = curl_easy_perform(session_.handle.get());
        if (canceling_) {
          return false;
        }
        if (exception_) {
          std::rethrow_exception(exception_);
        }
        check(code);

        if (!process_header_once()) {
          return false;
        }

        return true;
      }

    private:
      http_session_impl& session_;
      string_list header_;
      std::unique_ptr<http_reader> reader_;
      http_header_parser parser_;
      long code_;
      bool canceling_;
      std::exception_ptr exception_;

      static size_t read_cb(char* data, size_t size, size_t count, void* self) {
        return static_cast<http_task*>(self)->read(data, size * count);
      }

      static size_t header_cb(char* data, size_t size, size_t count, void* self) {
        return static_cast<http_task*>(self)->header(data, size * count);
      }

      static size_t write_cb(char* data, size_t size, size_t count, void* self) {
        return static_cast<http_task*>(self)->write(data, size * count);
      }

      template <class T>
      void setopt(CURLoption option, T parameter) {
        check(curl_easy_setopt(session_.handle.get(), option, parameter));
      }

      size_t read(char* data, size_t size) {
        try {
          if (reader_) {
            size_t result = reader_->read(data, size);
            if (!session_.progress_cb(reader_->now(), reader_->total())) {
              canceling_ = true;
              return CURL_READFUNC_ABORT;
            }
            return result;
          }
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return CURL_READFUNC_ABORT;
      }

      size_t header(const char* data, size_t size) {
        try {
          if (parser_.parse(data, size)) {
            check(curl_easy_getinfo(session_.handle.get(), CURLINFO_RESPONSE_CODE, &code_));
          }
          return size;
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return 0;
      }

      bool process_header_once() {
        if (code_ != -1) {
          long code = code_;
          code_ = -1;
          return session_.header_cb(code, parser_.get());
        }
        return true;
      }

      size_t write(const char* data, size_t size) {
        try {
          if (!process_header_once()) {
            canceling_ = true;
            return 0;
          }
          if (!session_.write_cb(data, size)) {
            canceling_ = true;
            return 0;
          }
          return size;
        } catch (...) {
          if (!exception_) {
            exception_ = std::current_exception();
          }
        }
        return 0;
      }
    };

    bool http_session_impl::request(
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& header,
        http_request_body body,
        const char* data,
        size_t size) {
      http_task task(*this);
      return task.request(method, url, header, body, data, size);
    }

    int http_initializer_count = 0;
  }

  http_initializer::http_initializer() {
    if (++http_initializer_count == 1) {
      curl_global_init(CURL_GLOBAL_ALL);
    }
  }

  http_initializer::~http_initializer() {
    if (--http_initializer_count == 0) {
      curl_global_cleanup();
    }
  }

  void open_http() {}

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (size_t, size_t)> progress_cb,
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      bool credential,
      const std::string& username,
      const std::string& password) {
    return std::unique_ptr<http_session>(new http_session_impl(progress_cb, header_cb, write_cb, credential, username, password));
  }
}
