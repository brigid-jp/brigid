// Copyright (c) 2019,2021,2022,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "crypto.hpp"
#include "error.hpp"
#include "scope_exit.hpp"
#include "stack_guard.hpp"
#include "thread_reference.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <utility>

namespace brigid {
  cryptor::cryptor(thread_reference&& ref)
    : in_size_(),
      out_size_(),
      ref_(std::move(ref)),
      running_() {}

  cryptor::~cryptor() {}

  void cryptor::update(const char* in_data, size_t in_size, bool padding) {
    in_size_ += in_size;
    ensure_buffer_size(impl_calculate_buffer_size(in_size_) - out_size_);
    size_t result = impl_update(in_data, in_size, buffer_.data(), buffer_.size(), padding);
    out_size_ += result;
    if (result > 0) {
      if (lua_State* L = ref_.get()) {
        stack_guard guard(L);
        lua_pushvalue(L, 1);
        view_t* view = new_view(L, buffer_.data(), result);
        running_ = true;
        scope_exit scope_guard([&]() {
          running_ = false;
          view->close();
        });
        if (lua_pcall(L, 1, 0, 0) != 0) {
          throw BRIGID_RUNTIME_ERROR(lua_tostring(L, -1));
        }
      }
    }
  }

  void cryptor::close() {
    in_size_ = 0;
    out_size_ = 0;
    ref_ = thread_reference();
  }

  bool cryptor::closed() const {
    return !ref_;
  }

  bool cryptor::running() const {
    return running_;
  }

  void cryptor::ensure_buffer_size(size_t size) {
    if (buffer_.size() < size) {
      buffer_.resize(size);
    }
  }

  hasher::~hasher() {}
}
