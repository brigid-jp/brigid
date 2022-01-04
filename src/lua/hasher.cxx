
#line 1 "hasher.rl"
// vim: syntax=ragel:

// Copyright (c) 2022 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "crypto.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"
#include "noncopyable.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace brigid {
  namespace {
    
#line 28 "hasher.cxx"
static const int hasher_name_chooser_start = 1;


#line 36 "hasher.rl"


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    hasher* new_hasher(lua_State* L, const char* name) {
      int cs = 0;
      
#line 43 "hasher.cxx"
	{
	cs = hasher_name_chooser_start;
	}

#line 46 "hasher.rl"
      const char* p = name;
      const char* pe = nullptr;
      
#line 52 "hasher.cxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 115 )
		goto st2;
	goto st0;
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( (*p) == 104 )
		goto st3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 97 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 49: goto st5;
		case 50: goto st6;
		case 53: goto st9;
	}
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 0 )
		goto tr7;
	goto st0;
tr7:
#line 29 "hasher.rl"
	{ return new_sha1_hasher(L); }
	goto st12;
tr10:
#line 31 "hasher.rl"
	{ return new_sha256_hasher(L); }
	goto st12;
tr13:
#line 33 "hasher.rl"
	{ return new_sha512_hasher(L); }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 112 "hasher.cxx"
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 53 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 54 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 0 )
		goto tr10;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 49 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 50 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 0 )
		goto tr13;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 49 "hasher.rl"
      return nullptr;
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    hasher* check_hasher(lua_State* L, int arg) {
      return check_udata<hasher>(L, arg, "brigid.hasher");
    }

    void impl_gc(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      self->~hasher();
    }

    void impl_call(lua_State* L) {
      const char* name = luaL_checkstring(L, 2);
      if (!new_hasher(L, name)) {
        luaL_argerror(L, 2, "unsupported hash");
      }
    }

    void impl_get_name(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      lua_pushstring(L, self->get_name());
    }

    void impl_update(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      data_t source = check_data(L, 2);
      self->update(source.data(), source.size());
    }

    void impl_digest(lua_State* L) {
      hasher* self = check_hasher(L, 1);
      std::vector<char> result = self->digest();
      lua_pushlstring(L, result.data(), result.size());
    }
  }

  void initialize_hasher(lua_State* L) {
    try {
      open_hasher();
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
      return;
    }

    lua_newtable(L);
    {
      new_metatable(L, "brigid.hasher");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_get_name>())::set_field(L, -1, "get_name");
      decltype(function<impl_update>())::set_field(L, -1, "update");
      decltype(function<impl_digest>())::set_field(L, -1, "digest");
    }
    lua_setfield(L, -2, "hasher");
  }
}
