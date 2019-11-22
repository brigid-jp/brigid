// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <Foundation/Foundation.h>

#include <string.h>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

namespace brigid {
  namespace {
    class http_session_context {
    public:
      http_session_context(int key)
        : key_(key),
          completed_() {}

      http_session_context(const http_session_context&) = delete;

      http_session_context& operator=(const http_session_context&) = delete;

      void debug(const std::string& message) {
        std::ostringstream out;
        out << key_ << " " << std::this_thread::get_id() << " " << message << "\n";
        std::cout << out.str();
      }

      void complete() {
        completed_ = true;
      }

      bool completed() const {
        return completed_;
      }

      std::mutex& mutex() {
        return mutex_;
      }

      std::condition_variable& cond() {
        return cond_;
      }

      void notify() {
        cond_.notify_one();
      }

      void status_code(int status_code) {
        status_code_ = status_code;
      }

      int status_code() const {
        return status_code_;
      }

      void copy(NSData* data) {
        __block size_t n = data_.size();
        size_t m = data.length;
        data_.resize(n + m);
        [data enumerateByteRangesUsingBlock:^(const void* bytes, NSRange byteRange, BOOL* stop) {
          std::cout << byteRange.length << "\n";
          memcpy(&data_[n], bytes, byteRange.length);
          n += byteRange.length;
        }];
      }

      const std::vector<char>& data() const {
        return data_;
      }

    private:
      int key_;
      std::mutex mutex_;
      std::condition_variable cond_;
      bool completed_;
      int status_code_;
      std::vector<char> data_;
    };
  }
}

@interface BrigidDelegate : NSObject <NSURLSessionDataDelegate>

@end

@implementation BrigidDelegate {
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
  std::lock_guard<std::mutex> lock(context_->mutex());
  context_->debug("didCompleteWithError");
  context_->complete();
  context_->notify();
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)response
 completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler {
  std::lock_guard<std::mutex> lock(context_->mutex());
  NSHTTPURLResponse* http_response = (NSHTTPURLResponse*) response;
  context_->debug("didReceiveResponse " + std::to_string(http_response.statusCode));
  context_->status_code(http_response.statusCode);
  context_->notify();
  completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
    didReceiveData:(NSData *)data {
  std::lock_guard<std::mutex> lock(context_->mutex());
  context_->debug("didReceiveData " + std::to_string(data.length));
  context_->copy(data);
  context_->notify();
}

@end

namespace brigid {
  NSString* make_string(const std::string& source) {
    return [[NSString alloc] initWithBytes:source.data() length:source.size() encoding:NSUTF8StringEncoding];
  }

  std::string to_string(NSString* source) {
    return std::string([source UTF8String], [source lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
  }

  std::string to_string(NSData* data) {
    return std::string(static_cast<const char*>(data.bytes), data.length);
  }

  void debug(int key, const std::string& message) {
    std::ostringstream out;
    out << key << " " << std::this_thread::get_id() << " " << message << "\n";
    std::cout << out.str();
  }

  namespace {
    class session {
    public:
      session(int key)
        : context_(std::make_shared<http_session_context>(key)) {}

      void http(const std::string& url) {
        NSURL* u = [[NSURL alloc] initWithString:make_string(url)];


        NSURLSessionConfiguration* configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
        BrigidDelegate* delegate = [[BrigidDelegate alloc] initWithContext:context_];
        NSURLSession* session = [NSURLSession sessionWithConfiguration:configuration delegate:delegate delegateQueue:nil];

        NSURLSessionTask* task = [session dataTaskWithURL:u];
        [task resume];

        {
          std::unique_lock<std::mutex> lock(context_->mutex());
          while (true) {
            context_->cond().wait(lock);
            context_->debug("wait");
            if (context_->completed()) {
              break;
            }
          }
        }

        [session invalidateAndCancel];
      }

      int status_code() const {
        return context_->status_code();
      }

      const std::vector<char>& data() const {
        return context_->data();
      }

    private:
      std::shared_ptr<http_session_context> context_;
    };
  }

  void http(int key, const std::string& url) {
    session s(key);
    s.http(url);

    std::ostringstream out;
    out << "done " << s.status_code() << " " << s.data().size();
    debug(key, out.str());
  }
}
