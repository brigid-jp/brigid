// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_DATA_HPP
#define BRIGID_DATA_HPP

#include <lua.hpp>

#include <stddef.h>
#include <string>

namespace brigid {
  class data_t {
  public:
    data_t();
    data_t(const char*, size_t);
    const char* data() const;
    size_t size() const;
    std::string str() const;
  private:
    const char* data_;
    size_t size_;
  };

  data_t to_data(lua_State*, int);
  data_t check_data(lua_State*, int);
}

#endif
