// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <Foundation/Foundation.h>

#include <iostream>
#include <sstream>
#include <thread>

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

  void http(int key, const std::string url) {
    {
      std::ostringstream out;
      out << "__has_feature(objc_arc) " << __has_feature(objc_arc);
      debug(key, out.str());
    }

    NSURL* u = [[NSURL alloc] initWithString:make_string(url)];

    NSURLSessionConfiguration* configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession* session = [NSURLSession sessionWithConfiguration:configuration];

    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    NSURLSessionTask* task = [session dataTaskWithURL:u completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
      if (error) {
        debug(key, "fail " + to_string(error.localizedDescription));
      } else {
        NSHTTPURLResponse* http_response = (NSHTTPURLResponse*) response;
        std::ostringstream out;
        out << "pass " << http_response.statusCode << " " << data.length;
        debug(key, out.str());
      }
      dispatch_semaphore_signal(semaphore);
    }];
    [task resume];

    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

    [session invalidateAndCancel];
  }
}
