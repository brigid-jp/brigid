// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_WRITER_HPP
#define BRIGID_WRITER_HPP

#include "common.hpp"

#include <lua.hpp>

#include <stddef.h>

namespace brigid {
  class writer_t {
  public:
    virtual ~writer_t() = 0;
    virtual bool closed() const = 0;
    virtual void write(const char*, size_t) = 0;
    virtual void write(char) = 0;
  };

  writer_t* to_data_writer(lua_State*, int);
  writer_t* to_file_writer(lua_State*, int);
  void initialize_writer(lua_State*);
}

#endif
