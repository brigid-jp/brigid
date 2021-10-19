// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_DATA_HPP
#define BRIGID_DATA_HPP

#include <lua.hpp>

#include <stddef.h>

namespace brigid {
  class abstract_data_t {
  public:
    virtual ~abstract_data_t() = 0;
    virtual bool closed() const = 0;
    virtual const char* data() const = 0;
    virtual size_t size() const = 0;
  };

  class data_t {
  public:
    data_t();
    data_t(const char*, size_t);
    const char* data() const;
    size_t size() const;
  private:
    const char* data_;
    size_t size_;
  };

  data_t to_data(lua_State*, int);
  data_t check_data(lua_State*, int);
}

#endif
