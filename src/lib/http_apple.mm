// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>

#include <Foundation/Foundation.h>

#include <condition_variable>
#include <memory>
#include <mutex>

namespace brigid {
  namespace {
    class http_session_context {
    public:
      http_session_context(
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          std::function<bool (size_t, size_t, size_t)> read_cb)
        : header_cb_(header_cb),
          write_cb_(write_cb),
          read_cb_(read_cb) {}

    private:
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
      std::function<bool (size_t, size_t, size_t)> read_cb_;
    };
  }
}

@interface BrigidSessionDelegate : NSObject <NSURLSessionDataDelegate> @end

@implementation BrigidSessionDelegate {
  std::shared_ptr<brigid::http_session_context> context_;
}

- (instancetype)initWithContext:(std::shared_ptr<brigid::http_session_context>)context {
  if (self = [super init]) {
    context_ = context;
  }
  return self;
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
didCompleteWithError:(NSError *)error {
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)response
 completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler {
  completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
    didReceiveData:(NSData *)data {
}

@end

namespace brigid {
  namespace {
    class http_session_impl : public http_session {
    public:
      http_session_impl(
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          std::function<bool (size_t, size_t, size_t)> read_cb)
        : context_(std::make_shared<http_session_context>(header_cb, write_cb, read_cb)),
          delegate_([[BrigidSessionDelegate alloc] initWithContext:context_]) {
      }

      ~http_session_impl() {
      }

      virtual void request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& headers,
          http_request_body body,
          const char* data,
          size_t size) {
      }

    private:
      std::shared_ptr<http_session_context> context_;
      BrigidSessionDelegate* delegate_;
    };
  }

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      std::function<bool (size_t, size_t, size_t)> read_cb) {
    return std::unique_ptr<http_session>(new http_session_impl(header_cb, write_cb, read_cb));
  }
}
