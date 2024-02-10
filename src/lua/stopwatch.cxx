
#line 1 "stopwatch.rl"
// vim: syntax=ragel:

// Copyright (c) 2021,2022 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "function.hpp"
#include "noncopyable.hpp"
#include "stopwatch.hpp"

#include <lua.hpp>

#include <stdint.h>
#include <chrono>

namespace brigid {
  namespace {
    char NAME_SYSTEM_CLOCK[] = "std::chrono::system_clock";
    char NAME_STEADY_CLOCK[] = "std::chrono::steady_clock";
    char NAME_HIGH_RESOLUTION_CLOCK[] = "std::chrono::high_resolution_clock";

    template <class T, const char* T_name>
    class stopwatch_chrono : public stopwatch, private noncopyable {
    public:
      virtual const char* get_name() const {
        return T_name;
      }

      virtual void start() {
        started_ = T::now();
      }

      virtual void stop() {
        stopped_ = T::now();
      }

      virtual int64_t get_elapsed() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(stopped_ - started_).count();
      }

    private:
      typename T::time_point started_;
      typename T::time_point stopped_;
    };

    template <class T, const char* T_name>
    stopwatch* new_stopwatch_chrono(lua_State* L) {
      return new_userdata<stopwatch_chrono<T, T_name> >(L, "brigid.stopwatch");
    }

    
#line 56 "stopwatch.cxx"
static const int stopwatch_name_chooser_start = 1;


#line 65 "stopwatch.rl"


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    stopwatch* new_stopwatch_chrono(lua_State* L, const char* name) {
      int cs = 0;
      
#line 71 "stopwatch.cxx"
	{
	cs = stopwatch_name_chooser_start;
	}

#line 75 "stopwatch.rl"
      const char* p = name;
      const char* pe = nullptr;
      
#line 80 "stopwatch.cxx"
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
	if ( (*p) == 116 )
		goto st3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 100 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 58 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 58 )
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 99 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 104 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 114 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 111 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 110 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 111 )
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 58 )
		goto st13;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 58 )
		goto st14;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 104: goto st15;
		case 115: goto st36;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 105 )
		goto st16;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 103 )
		goto st17;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 104 )
		goto st18;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 95 )
		goto st19;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( (*p) == 114 )
		goto st20;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( (*p) == 101 )
		goto st21;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) == 115 )
		goto st22;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( (*p) == 111 )
		goto st23;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 108 )
		goto st24;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( (*p) == 117 )
		goto st25;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) == 116 )
		goto st26;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( (*p) == 105 )
		goto st27;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( (*p) == 111 )
		goto st28;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( (*p) == 110 )
		goto st29;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( (*p) == 95 )
		goto st30;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 99 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( (*p) == 108 )
		goto st32;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( (*p) == 111 )
		goto st33;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 99 )
		goto st34;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 107 )
		goto st35;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 0 )
		goto tr36;
	goto st0;
tr36:
#line 61 "stopwatch.rl"
	{ return new_stopwatch_chrono<std::chrono::high_resolution_clock, NAME_HIGH_RESOLUTION_CLOCK>(L); }
	goto st59;
tr49:
#line 59 "stopwatch.rl"
	{ return new_stopwatch_chrono<std::chrono::steady_clock, NAME_STEADY_CLOCK>(L); }
	goto st59;
tr60:
#line 57 "stopwatch.rl"
	{ return new_stopwatch_chrono<std::chrono::system_clock, NAME_SYSTEM_CLOCK>(L); }
	goto st59;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
#line 349 "stopwatch.cxx"
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	switch( (*p) ) {
		case 116: goto st37;
		case 121: goto st48;
	}
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) == 101 )
		goto st38;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( (*p) == 97 )
		goto st39;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 100 )
		goto st40;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 121 )
		goto st41;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 95 )
		goto st42;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) == 99 )
		goto st43;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 108 )
		goto st44;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 111 )
		goto st45;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 99 )
		goto st46;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 107 )
		goto st47;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 0 )
		goto tr49;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 115 )
		goto st49;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 116 )
		goto st50;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 101 )
		goto st51;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 109 )
		goto st52;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( (*p) == 95 )
		goto st53;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 99 )
		goto st54;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 108 )
		goto st55;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 111 )
		goto st56;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 99 )
		goto st57;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 107 )
		goto st58;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 0 )
		goto tr60;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 78 "stopwatch.rl"
      return nullptr;
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    stopwatch* check_stopwatch(lua_State* L, int arg) {
      return check_udata<stopwatch>(L, arg, "brigid.stopwatch");
    }

    void impl_get_stopwatch_names(lua_State* L) {
      lua_newtable(L);
      int i = get_stopwatch_names(L, 0);

      lua_pushstring(L, NAME_SYSTEM_CLOCK);
      lua_rawseti(L, -2, ++i);
      lua_pushstring(L, NAME_STEADY_CLOCK);
      lua_rawseti(L, -2, ++i);
      lua_pushstring(L, NAME_HIGH_RESOLUTION_CLOCK);
      lua_rawseti(L, -2, ++i);
    }

    void impl_gc(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->~stopwatch();
    }

    void impl_call(lua_State* L) {
      const char* name = lua_tostring(L, 2);
      if (name) {
        if (!new_stopwatch(L, name) && !new_stopwatch_chrono(L, name)) {
          luaL_argerror(L, 2, "unsupported stopwatch");
        }
      } else {
        new_stopwatch(L);
      }
    }

    void impl_get_name(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      lua_pushstring(L, self->get_name());
    }

    void impl_start(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->start();
    }

    void impl_stop(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->stop();
    }

    void impl_get_elapsed(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      push_integer(L, self->get_elapsed());
    }

    int impl_pcall(lua_State* L) {
      stopwatch* self = check_stopwatch(L, 1);
      self->start();
      int result = lua_pcall(L, lua_gettop(L) - 2, LUA_MULTRET, 0);
      self->stop();
      if (result == 0) {
        lua_pushboolean(L, true);
        lua_replace(L, 1);
        return lua_gettop(L);
      } else {
        lua_pushboolean(L, false);
        lua_replace(L, 1);
        return 2;
      }
    }
  }

  stopwatch::~stopwatch() {}

  void initialize_stopwatch(lua_State* L) {
    decltype(function<impl_get_stopwatch_names>())::set_field(L, -1, "get_stopwatch_names");

    lua_newtable(L);
    {
      new_metatable(L, "brigid.stopwatch");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      lua_pop(L, 1);

      decltype(function<impl_call>())::set_metafield(L, -1, "__call");
      decltype(function<impl_get_name>())::set_field(L, -1, "get_name");
      decltype(function<impl_start>())::set_field(L, -1, "start");
      decltype(function<impl_stop>())::set_field(L, -1, "stop");
      decltype(function<impl_get_elapsed>())::set_field(L, -1, "get_elapsed");
      decltype(function<impl_pcall>())::set_field(L, -1, "pcall");
    }
    lua_setfield(L, -2, "stopwatch");
  }
}
