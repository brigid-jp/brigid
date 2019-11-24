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
    std::string to_string(NSString* source) {
      return std::string([source UTF8String], [source lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
    }

    struct http_session_context {
      http_session_context(
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          std::function<bool (size_t, size_t, size_t)> progress_cb)
        : header_cb(header_cb),
          write_cb(write_cb),
          progress_cb(progress_cb) {}

      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb;
      std::function<bool (const char*, size_t)> write_cb;
      std::function<bool (size_t, size_t, size_t)> progress_cb;

      std::mutex mutex;

      std::mutex req_mutex;
      std::condition_variable req_condition;
      std::function<bool ()> req_cb;
      // string
      NSError* req_error;

      std::mutex rep_mutex;
      std::condition_variable rep_condition;
      bool rep_result;
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

  std::lock_guard<std::mutex> lock(context_->mutex);

  {
    std::lock_guard<std::mutex> lock(context_->req_mutex);
    context_->req_cb = nullptr;
    context_->req_error = error;
    context_->req_condition.notify_all();
  }

  {
    std::unique_lock<std::mutex> lock(context_->rep_mutex);
    context_->rep_condition.wait(lock);
  }
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)response
 completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler {

  NSHTTPURLResponse* http_response = (NSHTTPURLResponse*) response;

  std::map<std::string, std::string> headers;
  for (NSString* key in http_response.allHeaderFields) {
    headers[brigid::to_string(key)] = brigid::to_string(http_response.allHeaderFields[key]);
  }

  std::lock_guard<std::mutex> lock(context_->mutex);

  {
    std::lock_guard<std::mutex> lock(context_->req_mutex);
    context_->req_cb = [&]() -> bool {
      return context_->header_cb(http_response.statusCode, headers);
    };
    context_->req_condition.notify_all();
  }

  {
    std::unique_lock<std::mutex> lock(context_->rep_mutex);
    context_->rep_condition.wait(lock);
    if (context_->rep_result) {
      completionHandler(NSURLSessionResponseAllow);
    } else {
      completionHandler(NSURLSessionResponseCancel);
    }
  }
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
    didReceiveData:(NSData *)data {

  std::lock_guard<std::mutex> lock(context_->mutex);

  __block bool result = true;

  [data enumerateByteRangesUsingBlock:^(const void* bytes, NSRange byteRange, BOOL* stop) {
    {
      std::lock_guard<std::mutex> lock(context_->req_mutex);
      context_->req_cb = [&]() -> bool {
        return context_->write_cb(static_cast<const char*>(bytes), byteRange.length);
      };
      context_->req_condition.notify_all();
    }

    {
      std::unique_lock<std::mutex> lock(context_->rep_mutex);
      context_->rep_condition.wait(lock);
      if (!context_->rep_result) {
        *stop = YES;
        result = false;
      }
    }
  }];

  if (!result) {
    [dataTask cancel];
  }
}

@end

namespace brigid {
  namespace {
    NSString* to_native_string(const std::string& source) {
      return [[NSString alloc] initWithBytes:source.data() length:source.size() encoding:NSUTF8StringEncoding];
    }

    NSString* to_native_string(const char* data, size_t size) {
      return [[NSString alloc] initWithBytes:data length:size encoding:NSUTF8StringEncoding];
    }

    class http_session_impl : public http_session {
    public:
      http_session_impl(
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          std::function<bool (size_t, size_t, size_t)> progress_cb)
        : context_(std::make_shared<http_session_context>(header_cb, write_cb, progress_cb)),
          delegate_([[BrigidSessionDelegate alloc] initWithContext:context_]),
          session_([NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration] delegate:delegate_ delegateQueue:nil]) {}

      virtual ~http_session_impl() {
        [session_ invalidateAndCancel];
      }

      virtual void request(
          const std::string& method,
          const std::string& url,
          const std::map<std::string, std::string>& headers,
          http_request_body body,
          const char* data,
          size_t size) {
        NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:to_native_string(url)]];
        request.HTTPMethod = to_native_string(method);
        for (const auto& header : headers) {
          [request setValue:to_native_string(header.second) forHTTPHeaderField:to_native_string(header.first)];
        }
        NSURLSessionTask* task = nil;
        switch (body) {
          case http_request_body::data:
            if (size != 0) {
              request.HTTPBody = [NSData dataWithBytes:data length:size];
            }
            task = [session_ dataTaskWithRequest:request];
            break;
          case http_request_body::file:
            task = [session_ uploadTaskWithRequest:request fromFile:[NSURL fileURLWithPath:to_native_string(data, size)]];
            break;
        }
        [task resume];

        std::unique_lock<std::mutex> lock(context_->rep_mutex);
        while (true) {
          context_->rep_condition.wait(lock);
          try {
          } catch (const std::exception& e) {
            // false;
          }
        }
      }

    private:
      std::shared_ptr<http_session_context> context_;
      BrigidSessionDelegate* delegate_;
      NSURLSession* session_;
    };
  }

  std::unique_ptr<http_session> make_http_session(
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
      std::function<bool (const char*, size_t)> write_cb,
      std::function<bool (size_t, size_t, size_t)> progress_cb) {
    return std::unique_ptr<http_session>(new http_session_impl(header_cb, write_cb, progress_cb));
  }
}
