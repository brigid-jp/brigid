
#line 1 "stopwatch_unix.rl"
// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "stopwatch.hpp"

#include <lua.hpp>

#include <stdint.h>
#include <time.h>

namespace brigid {
  namespace {
    char NAME_CLOCK_REALTIME[] = "CLOCK_REALTIME";
#ifdef CLOCK_REALTIME_COARSE
    char NAME_CLOCK_REALTIME_COARSE[] = "CLOCK_REALTIME_COARSE";
#endif
    char NAME_CLOCK_MONOTONIC[] = "CLOCK_MONOTONIC";
#ifdef CLOCK_MONOTONIC_COARSE
    char NAME_CLOCK_MONOTONIC_COARSE[] = "CLOCK_MONOTONIC_COARSE";
#endif
#ifdef CLOCK_MONOTONIC_RAW
    char NAME_CLOCK_MONOTONIC_RAW[] = "CLOCK_MONOTONIC_RAW";
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
    char NAME_CLOCK_MONOTONIC_RAW_APPROX[] = "CLOCK_MONOTONIC_RAW_APPROX";
#endif
#ifdef CLOCK_BOOTTIME
    char NAME_CLOCK_BOOTTIME[] = "CLOCK_BOOTTIME";
#endif
#ifdef CLOCK_UPTIME_RAW
    char NAME_CLOCK_UPTIME_RAW[] = "CLOCK_UPTIME_RAW";
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
    char NAME_CLOCK_UPTIME_RAW_APPROX[] = "CLOCK_UPTIME_RAW_APPROX";
#endif

    template <clockid_t T_clock, const char* T_name>
    class stopwatch_unix : public stopwatch, private noncopyable {
    public:
      stopwatch_unix()
        : started_(),
          stopped_() {}

      virtual const char* get_name() const {
        return T_name;
      }

      virtual void start() {
        if (clock_gettime(T_clock, &started_) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      virtual void stop() {
        if (clock_gettime(T_clock, &stopped_) == -1) {
          throw BRIGID_SYSTEM_ERROR();
        }
      }

      virtual int64_t get_elapsed() const {
        return (stopped_.tv_sec - started_.tv_sec) * 1000000000LL + stopped_.tv_nsec - started_.tv_nsec;
      }

    private:
      struct timespec started_;
      struct timespec stopped_;
    };

    template <clockid_t T_clock, const char* T_name>
    stopwatch* new_stopwatch_unix(lua_State* L) {
      return new_userdata<stopwatch_unix<T_clock, T_name> >(L, "brigid.stopwatch");
    }

    
#line 84 "stopwatch_unix.cxx"
static const int stopwatch_name_chooser_start = 1;


#line 151 "stopwatch_unix.rl"

  }

  stopwatch* new_stopwatch(lua_State* L) {
#ifdef CLOCK_MONOTONIC_RAW
    return new_stopwatch_unix<CLOCK_MONOTONIC_RAW, NAME_CLOCK_MONOTONIC_RAW>(L);
#else
    return new_stopwatch_unix<CLOCK_MONOTONIC, NAME_CLOCK_MONOTONIC>(L);
#endif
  }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

  stopwatch* new_stopwatch(lua_State* L, const char* name) {
    int cs = 0;
    
#line 108 "stopwatch_unix.cxx"
	{
	cs = stopwatch_name_chooser_start;
	}

#line 170 "stopwatch_unix.rl"
    const char* p = name;
    const char* pe = nullptr;
    
#line 117 "stopwatch_unix.cxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 67 )
		goto st2;
	goto st0;
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( (*p) == 76 )
		goto st3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 79 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 67 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 75 )
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 95 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 66: goto st8;
		case 77: goto st16;
		case 82: goto st42;
		case 85: goto st57;
	}
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 79 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 79 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 84 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 84 )
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 73 )
		goto st13;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 77 )
		goto st14;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 69 )
		goto st15;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 0 )
		goto tr18;
	goto st0;
tr18:
#line 125 "stopwatch_unix.rl"
	{
#ifdef CLOCK_BOOTTIME
            return new_stopwatch_unix<CLOCK_BOOTTIME, NAME_CLOCK_BOOTTIME>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
tr27:
#line 97 "stopwatch_unix.rl"
	{
            return new_stopwatch_unix<CLOCK_MONOTONIC, NAME_CLOCK_MONOTONIC>(L);
          }
	goto st74;
tr36:
#line 101 "stopwatch_unix.rl"
	{
#ifdef CLOCK_MONOTONIC_COARSE
            return new_stopwatch_unix<CLOCK_MONOTONIC_COARSE, NAME_CLOCK_MONOTONIC_COARSE>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
tr39:
#line 109 "stopwatch_unix.rl"
	{
#ifdef CLOCK_MONOTONIC_RAW
            return new_stopwatch_unix<CLOCK_MONOTONIC_RAW, NAME_CLOCK_MONOTONIC_RAW>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
tr47:
#line 117 "stopwatch_unix.rl"
	{
#ifdef CLOCK_MONOTONIC_RAW_APPROX
            return new_stopwatch_unix<CLOCK_MONOTONIC_RAW_APPROX, NAME_CLOCK_MONOTONIC_RAW_APPROX>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
tr55:
#line 85 "stopwatch_unix.rl"
	{
            return new_stopwatch_unix<CLOCK_REALTIME, NAME_CLOCK_REALTIME>(L);
          }
	goto st74;
tr63:
#line 89 "stopwatch_unix.rl"
	{
#ifdef CLOCK_REALTIME_COARSE
            return new_stopwatch_unix<CLOCK_REALTIME_COARSE, NAME_CLOCK_REALTIME_COARSE>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
tr73:
#line 133 "stopwatch_unix.rl"
	{
#ifdef CLOCK_UPTIME_RAW
            return new_stopwatch_unix<CLOCK_UPTIME_RAW, NAME_CLOCK_UPTIME_RAW>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
tr81:
#line 141 "stopwatch_unix.rl"
	{
#ifdef CLOCK_UPTIME_RAW_APPROX
            return new_stopwatch_unix<CLOCK_UPTIME_RAW_APPROX, NAME_CLOCK_UPTIME_RAW_APPROX>(L);
#else
            return nullptr;
#endif
          }
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 318 "stopwatch_unix.cxx"
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 79 )
		goto st17;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 78 )
		goto st18;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 79 )
		goto st19;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( (*p) == 84 )
		goto st20;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( (*p) == 79 )
		goto st21;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) == 78 )
		goto st22;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( (*p) == 73 )
		goto st23;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 67 )
		goto st24;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 0: goto tr27;
		case 95: goto st25;
	}
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	switch( (*p) ) {
		case 67: goto st26;
		case 82: goto st32;
	}
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( (*p) == 79 )
		goto st27;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( (*p) == 65 )
		goto st28;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( (*p) == 82 )
		goto st29;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( (*p) == 83 )
		goto st30;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 69 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( (*p) == 0 )
		goto tr36;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( (*p) == 65 )
		goto st33;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( (*p) == 87 )
		goto st34;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	switch( (*p) ) {
		case 0: goto tr39;
		case 95: goto st35;
	}
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 65 )
		goto st36;
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	if ( (*p) == 80 )
		goto st37;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) == 80 )
		goto st38;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( (*p) == 82 )
		goto st39;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 79 )
		goto st40;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( (*p) == 88 )
		goto st41;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 0 )
		goto tr47;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) == 69 )
		goto st43;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 65 )
		goto st44;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 76 )
		goto st45;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 84 )
		goto st46;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 73 )
		goto st47;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 77 )
		goto st48;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 69 )
		goto st49;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 0: goto tr55;
		case 95: goto st50;
	}
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 67 )
		goto st51;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( (*p) == 79 )
		goto st52;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( (*p) == 65 )
		goto st53;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 82 )
		goto st54;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 83 )
		goto st55;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 69 )
		goto st56;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 0 )
		goto tr63;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 80 )
		goto st58;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 84 )
		goto st59;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 73 )
		goto st60;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 77 )
		goto st61;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 69 )
		goto st62;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 95 )
		goto st63;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 82 )
		goto st64;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 65 )
		goto st65;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 87 )
		goto st66;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 0: goto tr73;
		case 95: goto st67;
	}
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 65 )
		goto st68;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 80 )
		goto st69;
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) == 80 )
		goto st70;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 82 )
		goto st71;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 79 )
		goto st72;
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 88 )
		goto st73;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 0 )
		goto tr81;
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
	_test_eof74: cs = 74; goto _test_eof; 
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
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 173 "stopwatch_unix.rl"
    return nullptr;
  }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

  int get_stopwatch_names(lua_State* L, int i) {
    lua_pushstring(L, NAME_CLOCK_REALTIME);
    lua_rawseti(L, -2, ++i);

#ifdef CLOCK_REALTIME_COARSE
    lua_pushstring(L, NAME_CLOCK_REALTIME_COARSE);
    lua_rawseti(L, -2, ++i);
#endif

    lua_pushstring(L, NAME_CLOCK_MONOTONIC);
    lua_rawseti(L, -2, ++i);

#ifdef CLOCK_MONOTONIC_COARSE
    lua_pushstring(L, NAME_CLOCK_MONOTONIC_COARSE);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_MONOTONIC_RAW
    lua_pushstring(L, NAME_CLOCK_MONOTONIC_RAW);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_MONOTONIC_RAW_APPROX
    lua_pushstring(L, NAME_CLOCK_MONOTONIC_RAW_APPROX);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_BOOTTIME
    lua_pushstring(L, NAME_CLOCK_BOOTTIME);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_UPTIME_RAW
    lua_pushstring(L, NAME_CLOCK_UPTIME_RAW);
    lua_rawseti(L, -2, ++i);
#endif

#ifdef CLOCK_UPTIME_RAW_APPROX
    lua_pushstring(L, NAME_CLOCK_UPTIME_RAW_APPROX);
    lua_rawseti(L, -2, ++i);
#endif

    return i;
  }
}
