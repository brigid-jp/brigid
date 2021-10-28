// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <lua.hpp>

#include <exception>

namespace brigid {
  void initialize_common(lua_State*);
  void initialize_cryptor(lua_State*);
  void initialize_data_writer(lua_State*);
  void initialize_file_writer(lua_State*);
  void initialize_hasher(lua_State*);
  void initialize_http(lua_State*);
  void initialize_json(lua_State*);
  void initialize_stopwatch(lua_State*);
  void initialize_view(lua_State*);

  void initialize(lua_State* L) {
    initialize_common(L);
    initialize_cryptor(L);
    initialize_data_writer(L);
    initialize_file_writer(L);
    initialize_hasher(L);
    initialize_http(L);
    initialize_json(L);
    initialize_stopwatch(L);
    initialize_view(L);
  }
}

extern "C" int luaopen_brigid(lua_State* L) {
  int top = lua_gettop(L);
  try {
    lua_newtable(L);
    brigid::initialize(L);
    return 1;
  } catch (const std::exception& e) {
    lua_settop(L, top);
    return luaL_error(L, "%s", e.what());
  } catch (...) {
    lua_settop(L, top);
    return luaL_error(L, "unknown exception");
  }
}
