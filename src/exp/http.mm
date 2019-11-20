// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "http.hpp"

#include <Foundation/Foundation.h>

#include <iostream>

namespace brigid {
  NSString* make_string(const std::string& source) {
    return [[[NSString alloc] initWithBytes:source.data() length:source.size() encoding:NSUTF8StringEncoding] autorelease];
  }

  std::string to_string(NSString* source) {
    return std::string([source UTF8String], [source lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
  }

  std::string to_string(NSData* data) {
    return std::string(static_cast<const char*>(data.bytes), data.length);
  }

  void http(const std::string url) {
    std::cout << url << "\n";

    NSURL* u = [[[NSURL alloc] initWithString:make_string(url)] autorelease];

    std::cout << to_string(u.scheme) << "\n";
    std::cout << to_string(u.host) << "\n";

    NSURLSessionConfiguration* configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession* session = [NSURLSession sessionWithConfiguration:configuration];

    dispatch_semaphore_t semaphore = [dispatch_semaphore_create(0) autorelease];

    NSURLSessionTask* task = [session dataTaskWithURL:u completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
      if (error) {
        std::cout << "fail " << to_string(error.localizedDescription) << "\n";
      } else {
        NSHTTPURLResponse* http_response = (NSHTTPURLResponse*) response;
        std::cout << "pass " << http_response.statusCode << " " << data.length << "\n";
      }
      dispatch_semaphore_signal(semaphore);
    }];
    [task resume];

    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  }
}
