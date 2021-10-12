// Copyright (c) 2019,2020 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_VIEW_HPP
#define BRIGID_VIEW_HPP

#include <brigid/noncopyable.hpp>
#include "data.hpp"

#include <lua.hpp>

#include <stddef.h>

namespace brigid {
  class view_t : public abstract_data_t, private noncopyable {
  public:
    view_t(const char*, size_t);
    virtual bool closed() const;
    virtual const char* data() const;
    virtual size_t size() const;
    void close();
  private:
    const char* data_;
    size_t size_;
  };

  view_t* new_view(lua_State*, const char*, size_t);
}

#endif
