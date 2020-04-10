// Copyright (c) 2019,2020 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <lua.hpp>

namespace brigid {
  void initialize_common(lua_State*);
  void initialize_cryptor(lua_State*);
  void initialize_data_writer(lua_State*);
  void initialize_file_writer(lua_State*);
  void initialize_hasher(lua_State*);
  void initialize_http(lua_State*);
  void initialize_version(lua_State*);
  void initialize_view(lua_State*);

  void initialize(lua_State* L) {
    initialize_common(L);
    initialize_cryptor(L);
    initialize_data_writer(L);
    initialize_file_writer(L);
    initialize_hasher(L);
    initialize_http(L);
    initialize_version(L);
    initialize_view(L);
  }
}

extern "C" int luaopen_brigid(lua_State* L) {
  lua_newtable(L);
  brigid::initialize(L);
  return 1;
}
