// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/http.hpp>
#include <brigid/noncopyable.hpp>

#include <Foundation/Foundation.h>

#include <condition_variable>
#include <exception>
#include <mutex>
#include <stdexcept>

namespace brigid {
  http_initializer::http_initializer() {}
  http_initializer::~http_initializer() {}

  namespace {
    class http_session_delegate_impl : private noncopyable {
    public:
      http_session_delegate_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : progress_cb_(progress_cb),
          header_cb_(header_cb),
          write_cb_(write_cb),
          credential_(credential),
          username_(username),
          password_(password),
          rep_() {}
      void did_complete_with_error(NSError*);
      bool did_send_body_data(size_t, size_t);
      NSURLCredential* did_receive_challenge(NSURLAuthenticationChallenge*);
      bool did_receive_response(NSHTTPURLResponse*);
      bool did_receive_data(NSData*);
      void wait(NSURLSessionTask*);

    private:
      std::function<bool (size_t, size_t)> progress_cb_;
      std::function<bool (int, const std::map<std::string, std::string>&)> header_cb_;
      std::function<bool (const char*, size_t)> write_cb_;
      bool credential_;
      std::string username_;
      std::string password_;
      std::mutex req_mutex_;
      std::condition_variable req_condition_;
      std::function<bool ()> req_;
      std::mutex rep_mutex_;
      std::condition_variable rep_condition_;
      bool rep_;
      std::exception_ptr exception_;
    };
  }
}

@interface BrigidHttpSessionDelegate : NSObject <NSURLSessionDataDelegate> @end

@implementation BrigidHttpSessionDelegate {
  std::shared_ptr<brigid::http_session_delegate_impl> impl_;
}

- (instancetype)initWithImpl:(std::shared_ptr<brigid::http_session_delegate_impl>)impl {
  if (self = [super init]) {
    impl_ = impl;
  }
  return self;
}

- (void)URLSession:(NSURLSession *)_session
              task:(NSURLSessionTask *)task
didCompleteWithError:(NSError *)error {
  impl_->did_complete_with_error(error);
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
   didSendBodyData:(int64_t)bytesSent
    totalBytesSent:(int64_t)totalBytesSent
totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend {
  if (!impl_->did_send_body_data(totalBytesSent, totalBytesExpectedToSend)) {
    [task cancel];
  }
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
 completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *credential))completionHandler {
  if (NSURLCredential* credential = impl_->did_receive_challenge(challenge)) {
    completionHandler(NSURLSessionAuthChallengeUseCredential, credential);
  } else {
    completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, nil);
  }
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)response
 completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler {
  if (impl_->did_receive_response((NSHTTPURLResponse*) response)) {
    completionHandler(NSURLSessionResponseAllow);
  } else {
    completionHandler(NSURLSessionResponseCancel);
  }
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
    didReceiveData:(NSData *)data {
  if (!impl_->did_receive_data(data)) {
    [dataTask cancel];
  }
}

@end

namespace brigid {
  namespace {
    std::string to_string(NSString* source) {
      return std::string([source UTF8String], [source lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
    }

    NSString* to_native_string(const std::string& source) {
      return [[NSString alloc] initWithBytes:source.data() length:source.size() encoding:NSUTF8StringEncoding];
    }

    NSString* to_native_string(const char* data, size_t size) {
      return [[NSString alloc] initWithBytes:data length:size encoding:NSUTF8StringEncoding];
    }

    void http_session_delegate_impl::did_complete_with_error(NSError* error) {
      std::lock_guard<std::mutex> req_lock(req_mutex_);
      req_ = nullptr;
      if (error && !exception_) {
        exception_ = std::make_exception_ptr(std::runtime_error(to_string(error.localizedDescription)));
      }
      req_condition_.notify_all();
    }

    bool http_session_delegate_impl::did_send_body_data(size_t total_bytes_sent, size_t total_bytes_expected_to_send) {
      if (progress_cb_) {
        std::unique_lock<std::mutex> rep_lock(rep_mutex_, std::defer_lock);
        {
          std::lock_guard<std::mutex> req_lock(req_mutex_);
          req_ = [=]() -> bool {
            return progress_cb_(total_bytes_sent, total_bytes_expected_to_send);
          };
          rep_lock.lock();
          req_condition_.notify_all();
        }
        rep_condition_.wait(rep_lock);
        return rep_;
      } else {
        return true;
      }
    }

    NSURLCredential* http_session_delegate_impl::did_receive_challenge(NSURLAuthenticationChallenge* challenge) {
      if (challenge.previousFailureCount == 0 && credential_) {
        return [NSURLCredential credentialWithUser:to_native_string(username_) password:to_native_string(password_) persistence:NSURLCredentialPersistenceForSession];
      } else {
        return nil;
      }
    }

    bool http_session_delegate_impl::did_receive_response(NSHTTPURLResponse* response) {
      if (header_cb_) {
        std::unique_lock<std::mutex> rep_lock(rep_mutex_, std::defer_lock);
        {
          std::lock_guard<std::mutex> req_lock(req_mutex_);
          req_ = [=]() -> bool {
            std::map<std::string, std::string> headers;
            for (NSString* key in response.allHeaderFields) {
              headers[brigid::to_string(key)] = brigid::to_string(response.allHeaderFields[key]);
            }
            return header_cb_(response.statusCode, headers);
          };
          rep_lock.lock();
          req_condition_.notify_all();
        }
        rep_condition_.wait(rep_lock);
        return rep_;
      } else {
        return true;
      }
    }

    bool http_session_delegate_impl::did_receive_data(NSData* data) {
      if (write_cb_) {
        std::unique_lock<std::mutex> rep_lock(rep_mutex_, std::defer_lock);
        {
          std::lock_guard<std::mutex> req_lock(req_mutex_);
          req_ = [=]() -> bool {
            __block bool result = true;
            [data enumerateByteRangesUsingBlock:^(const void* bytes, NSRange byteRange, BOOL* stop) {
              result = write_cb_(static_cast<const char*>(bytes), byteRange.length);
              if (!result) {
                *stop = YES;
              }
            }];
            return result;
          };
          rep_lock.lock();
          req_condition_.notify_all();
        }
        rep_condition_.wait(rep_lock);
        return rep_;
      } else {
        return true;
      }
    }

    void http_session_delegate_impl::wait(NSURLSessionTask* task) {
      {
        std::unique_lock<std::mutex> req_lock(req_mutex_);

        [task resume];

        while (true) {
          req_condition_.wait(req_lock);
          std::function<bool ()> req = req_;
          req_ = nullptr;
          rep_ = false;
          if (req) {
            try {
              rep_ = req();
            } catch (...) {
              if (!exception_) {
                exception_ = std::current_exception();
              }
            }
          } else {
            break;
          }

          std::lock_guard<std::mutex> rep_lock(rep_mutex_);
          rep_condition_.notify_all();
        }
      }

      if (exception_) {
        std::exception_ptr exception = exception_;
        exception_ = nullptr;
        std::rethrow_exception(exception);
      }
    }

    class http_session_impl : public http_session {
    public:
      http_session_impl(
          std::function<bool (size_t, size_t)> progress_cb,
          std::function<bool (int, const std::map<std::string, std::string>&)> header_cb,
          std::function<bool (const char*, size_t)> write_cb,
          bool credential,
          const std::string& username,
          const std::string& password)
        : impl_(std::make_shared<http_session_delegate_impl>(progress_cb, header_cb, write_cb, credential, username, password)),
          session_([NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration ephemeralSessionConfiguration] delegate:[[BrigidHttpSessionDelegate alloc] initWithImpl:impl_] delegateQueue:nil]) {}

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
            if (data) {
              request.HTTPBody = [NSData dataWithBytes:data length:size];
            }
            task = [session_ dataTaskWithRequest:request];
            break;
          case http_request_body::file:
            task = [session_ uploadTaskWithRequest:request fromFile:[NSURL fileURLWithPath:to_native_string(data, size)]];
            break;
        }
        impl_->wait(task);
      }

    private:
      std::shared_ptr<http_session_delegate_impl> impl_;
      NSURLSession* session_;
    };
  }

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
