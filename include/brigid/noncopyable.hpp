// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_NONCOPYABLE_HPP
#define BRIGID_NONCOPYABLE_HPP

namespace brigid {
  class noncopyable {
  public:
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
  };
}

#endif
