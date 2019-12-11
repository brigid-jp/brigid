// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_SCOPE_EXIT_HPP
#define BRIGID_SCOPE_EXIT_HPP

#include <brigid/noncopyable.hpp>

#include <functional>

namespace brigid {
  class scope_exit : private noncopyable {
  public:
    explicit scope_exit(std::function<void ()>);
    ~scope_exit();
  private:
    std::function<void ()> function_;
  };
}

#endif
