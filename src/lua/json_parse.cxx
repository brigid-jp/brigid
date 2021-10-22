
#line 1 "json_parse.rl"
// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <sstream>
#include <type_traits>
#include <vector>

namespace brigid {
  using lua_unsigned_t = std::make_unsigned<lua_Integer>::type;
  static const size_t integer_digs = std::numeric_limits<lua_Integer>::digits10 + 1;
  static const lua_unsigned_t integer_max_div10 = std::numeric_limits<lua_Integer>::max() / 10;
  static const lua_unsigned_t integer_max_mod10 = std::numeric_limits<lua_Integer>::max() % 10;

  namespace {
    
#line 32 "json_parse.cxx"
static const int json_parser_start = 1;


#line 219 "json_parse.rl"


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    void impl_parse(lua_State* L) {
      data_t data = check_data(L, 1);

      int cs = 0;
      int top = lua_gettop(L);

      int null_index = top >= 2 ? 2 : 0;
      luaL_getmetatable(L, "brigid.json.array");
      int array_index = top + 1;

      
#line 55 "json_parse.cxx"
	{
	cs = json_parser_start;
	top = 0;
	}

#line 237 "json_parse.rl"

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();
      const char* const eof = pe;
      std::vector<int> stack; stack.reserve(16);

      const char* ps = nullptr;
      std::vector<char> buffer;
      std::vector<int> array_stack;
      bool is_int = false;    // number is integer
      char decimal_point = 0; // *localeconv()->decimal_point
      uint32_t u = 0;         // unicode escape sequence

      
#line 77 "json_parse.cxx"
	{
	if ( p == pe )
		goto _test_eof;
	goto _resume;

_again:
	switch ( cs ) {
		case 1: goto st1;
		case 0: goto st0;
		case 2: goto st2;
		case 3: goto st3;
		case 88: goto st88;
		case 4: goto st4;
		case 89: goto st89;
		case 5: goto st5;
		case 90: goto st90;
		case 6: goto st6;
		case 7: goto st7;
		case 91: goto st91;
		case 92: goto st92;
		case 8: goto st8;
		case 9: goto st9;
		case 10: goto st10;
		case 11: goto st11;
		case 12: goto st12;
		case 13: goto st13;
		case 14: goto st14;
		case 15: goto st15;
		case 16: goto st16;
		case 17: goto st17;
		case 18: goto st18;
		case 19: goto st19;
		case 20: goto st20;
		case 93: goto st93;
		case 21: goto st21;
		case 22: goto st22;
		case 23: goto st23;
		case 24: goto st24;
		case 25: goto st25;
		case 26: goto st26;
		case 27: goto st27;
		case 28: goto st28;
		case 29: goto st29;
		case 30: goto st30;
		case 31: goto st31;
		case 32: goto st32;
		case 33: goto st33;
		case 34: goto st34;
		case 35: goto st35;
		case 36: goto st36;
		case 37: goto st37;
		case 38: goto st38;
		case 39: goto st39;
		case 40: goto st40;
		case 41: goto st41;
		case 42: goto st42;
		case 43: goto st43;
		case 44: goto st44;
		case 45: goto st45;
		case 94: goto st94;
		case 46: goto st46;
		case 47: goto st47;
		case 48: goto st48;
		case 49: goto st49;
		case 50: goto st50;
		case 51: goto st51;
		case 52: goto st52;
		case 53: goto st53;
		case 54: goto st54;
		case 55: goto st55;
		case 56: goto st56;
		case 57: goto st57;
		case 58: goto st58;
		case 59: goto st59;
		case 60: goto st60;
		case 61: goto st61;
		case 62: goto st62;
		case 63: goto st63;
		case 64: goto st64;
		case 65: goto st65;
		case 66: goto st66;
		case 67: goto st67;
		case 68: goto st68;
		case 69: goto st69;
		case 70: goto st70;
		case 71: goto st71;
		case 72: goto st72;
		case 73: goto st73;
		case 74: goto st74;
		case 75: goto st75;
		case 76: goto st76;
		case 95: goto st95;
		case 77: goto st77;
		case 78: goto st78;
		case 79: goto st79;
		case 80: goto st80;
		case 81: goto st81;
		case 82: goto st82;
		case 83: goto st83;
		case 84: goto st84;
		case 85: goto st85;
		case 86: goto st86;
		case 87: goto st87;
	default: break;
	}

	if ( ++p == pe )
		goto _test_eof;
_resume:
	switch ( cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 13: goto st1;
		case 32: goto st1;
		case 34: goto tr2;
		case 45: goto tr3;
		case 48: goto tr4;
		case 91: goto tr6;
		case 102: goto st8;
		case 110: goto st12;
		case 116: goto st15;
		case 123: goto tr10;
	}
	if ( (*p) > 10 ) {
		if ( 49 <= (*p) && (*p) <= 57 )
			goto tr5;
	} else if ( (*p) >= 9 )
		goto st1;
	goto st0;
st0:
cs = 0;
	goto _out;
tr2:
#line 193 "json_parse.rl"
	{ ps = p + 1; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 222 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr12;
		case 92: goto tr13;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 34: goto tr14;
		case 92: goto tr15;
	}
	goto st3;
tr6:
#line 207 "json_parse.rl"
	{ lua_checkstack(L, 2); lua_createtable(L, 8, 0); array_stack.push_back(0); { stack.push_back(0); {stack[top++] = 88;goto st64;}} }
	goto st88;
tr10:
#line 206 "json_parse.rl"
	{ lua_checkstack(L, 3); lua_createtable(L, 0, 8); { stack.push_back(0); {stack[top++] = 88;goto st36;}} }
	goto st88;
tr12:
#line 194 "json_parse.rl"
	{ lua_pushlstring(L, ps, 0); }
	goto st88;
tr13:
#line 199 "json_parse.rl"
	{ buffer.clear(); { stack.push_back(0); {stack[top++] = 88;goto st18;}} }
	goto st88;
tr14:
#line 196 "json_parse.rl"
	{ lua_pushlstring(L, ps, p - ps); }
	goto st88;
tr15:
#line 197 "json_parse.rl"
	{ size_t n = p - ps; buffer.resize(n); memcpy(buffer.data(), ps, n); { stack.push_back(0); {stack[top++] = 88;goto st18;}} }
	goto st88;
tr24:
#line 203 "json_parse.rl"
	{ lua_pushboolean(L, false); }
	goto st88;
tr27:
#line 204 "json_parse.rl"
	{ if (null_index) { lua_pushvalue(L, null_index); } else { lua_pushnil(L); } }
	goto st88;
tr30:
#line 205 "json_parse.rl"
	{ lua_pushboolean(L, true); }
	goto st88;
tr180:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
	goto st88;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
#line 364 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto st88;
		case 32: goto st88;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st88;
	goto st0;
tr3:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 380 "json_parse.cxx"
	if ( (*p) == 48 )
		goto st89;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto st92;
	goto st0;
tr4:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st89;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
#line 394 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr180;
		case 32: goto tr180;
		case 46: goto tr181;
		case 69: goto tr182;
		case 101: goto tr182;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr180;
	goto st0;
tr181:
#line 38 "json_parse.rl"
	{ is_int = false; }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 413 "json_parse.cxx"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st90;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 13: goto tr180;
		case 32: goto tr180;
		case 69: goto st6;
		case 101: goto st6;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st90;
	} else if ( (*p) >= 9 )
		goto tr180;
	goto st0;
tr182:
#line 39 "json_parse.rl"
	{ is_int = false; }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 441 "json_parse.cxx"
	switch( (*p) ) {
		case 43: goto st7;
		case 45: goto st7;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st91;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st91;
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	switch( (*p) ) {
		case 13: goto tr180;
		case 32: goto tr180;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st91;
	} else if ( (*p) >= 9 )
		goto tr180;
	goto st0;
tr5:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
#line 478 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr180;
		case 32: goto tr180;
		case 46: goto tr181;
		case 69: goto tr182;
		case 101: goto tr182;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st92;
	} else if ( (*p) >= 9 )
		goto tr180;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 97 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 108 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 115 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 101 )
		goto tr24;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 117 )
		goto st13;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 108 )
		goto st14;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 108 )
		goto tr27;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 114 )
		goto st16;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 117 )
		goto st17;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 101 )
		goto tr30;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 34: goto tr31;
		case 47: goto tr32;
		case 92: goto tr33;
		case 98: goto tr34;
		case 102: goto tr35;
		case 110: goto tr36;
		case 114: goto tr37;
		case 116: goto tr38;
		case 117: goto tr39;
	}
	goto st0;
tr31:
#line 171 "json_parse.rl"
	{ buffer.push_back('"'); }
	goto st19;
tr32:
#line 173 "json_parse.rl"
	{ buffer.push_back('/'); }
	goto st19;
tr33:
#line 172 "json_parse.rl"
	{ buffer.push_back('\\'); }
	goto st19;
tr34:
#line 174 "json_parse.rl"
	{ buffer.push_back('\b'); }
	goto st19;
tr35:
#line 175 "json_parse.rl"
	{ buffer.push_back('\f'); }
	goto st19;
tr36:
#line 176 "json_parse.rl"
	{ buffer.push_back('\n'); }
	goto st19;
tr37:
#line 177 "json_parse.rl"
	{ buffer.push_back('\r'); }
	goto st19;
tr38:
#line 178 "json_parse.rl"
	{ buffer.push_back('\t'); }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 614 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr41;
		case 92: goto tr42;
	}
	goto tr40;
tr40:
#line 183 "json_parse.rl"
	{ ps = p; }
	goto st20;
tr60:
#line 141 "json_parse.rl"
	{
              if (u <= 0x007F) {
                buffer.push_back(u);
              } else if (u <= 0x07FF) {
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xC0);
                buffer.push_back(u2 | 0x80);
              } else {
                uint8_t u3 = u & 0x3F; u >>= 6;
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xE0);
                buffer.push_back(u2 | 0x80);
                buffer.push_back(u3 | 0x80);
              }
            }
#line 183 "json_parse.rl"
	{ ps = p; }
	goto st20;
tr84:
#line 158 "json_parse.rl"
	{
              u = ((u >> 16) - 0xD800) << 10 | ((u & 0xFFFF) - 0xDC00) | 0x010000;
              uint8_t u4 = u & 0x3F; u >>= 6;
              uint8_t u3 = u & 0x3F; u >>= 6;
              uint8_t u2 = u & 0x3F; u >>= 6;
              buffer.push_back(u  | 0xF0);
              buffer.push_back(u2 | 0x80);
              buffer.push_back(u3 | 0x80);
              buffer.push_back(u4 | 0x80);
            }
#line 183 "json_parse.rl"
	{ ps = p; }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 663 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr44;
		case 92: goto tr45;
	}
	goto st20;
tr41:
#line 183 "json_parse.rl"
	{ ps = p; }
#line 184 "json_parse.rl"
	{ lua_pushlstring(L, buffer.data(), buffer.size()); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st93;
tr42:
#line 183 "json_parse.rl"
	{ ps = p; }
#line 189 "json_parse.rl"
	{ {goto st18;} }
	goto st93;
tr44:
#line 186 "json_parse.rl"
	{ size_t m = buffer.size(); size_t n = p - ps; buffer.resize(m + n); char* ptr = buffer.data(); memcpy(ptr + m, ps, n); lua_pushlstring(L, ptr, m + n); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st93;
tr45:
#line 187 "json_parse.rl"
	{ size_t m = buffer.size(); size_t n = p - ps; buffer.resize(m + n); memcpy(buffer.data() + m, ps, n); {goto st18;} }
	goto st93;
tr61:
#line 141 "json_parse.rl"
	{
              if (u <= 0x007F) {
                buffer.push_back(u);
              } else if (u <= 0x07FF) {
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xC0);
                buffer.push_back(u2 | 0x80);
              } else {
                uint8_t u3 = u & 0x3F; u >>= 6;
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xE0);
                buffer.push_back(u2 | 0x80);
                buffer.push_back(u3 | 0x80);
              }
            }
#line 183 "json_parse.rl"
	{ ps = p; }
#line 184 "json_parse.rl"
	{ lua_pushlstring(L, buffer.data(), buffer.size()); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st93;
tr62:
#line 141 "json_parse.rl"
	{
              if (u <= 0x007F) {
                buffer.push_back(u);
              } else if (u <= 0x07FF) {
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xC0);
                buffer.push_back(u2 | 0x80);
              } else {
                uint8_t u3 = u & 0x3F; u >>= 6;
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xE0);
                buffer.push_back(u2 | 0x80);
                buffer.push_back(u3 | 0x80);
              }
            }
#line 183 "json_parse.rl"
	{ ps = p; }
#line 189 "json_parse.rl"
	{ {goto st18;} }
	goto st93;
tr85:
#line 158 "json_parse.rl"
	{
              u = ((u >> 16) - 0xD800) << 10 | ((u & 0xFFFF) - 0xDC00) | 0x010000;
              uint8_t u4 = u & 0x3F; u >>= 6;
              uint8_t u3 = u & 0x3F; u >>= 6;
              uint8_t u2 = u & 0x3F; u >>= 6;
              buffer.push_back(u  | 0xF0);
              buffer.push_back(u2 | 0x80);
              buffer.push_back(u3 | 0x80);
              buffer.push_back(u4 | 0x80);
            }
#line 183 "json_parse.rl"
	{ ps = p; }
#line 184 "json_parse.rl"
	{ lua_pushlstring(L, buffer.data(), buffer.size()); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st93;
tr86:
#line 158 "json_parse.rl"
	{
              u = ((u >> 16) - 0xD800) << 10 | ((u & 0xFFFF) - 0xDC00) | 0x010000;
              uint8_t u4 = u & 0x3F; u >>= 6;
              uint8_t u3 = u & 0x3F; u >>= 6;
              uint8_t u2 = u & 0x3F; u >>= 6;
              buffer.push_back(u  | 0xF0);
              buffer.push_back(u2 | 0x80);
              buffer.push_back(u3 | 0x80);
              buffer.push_back(u4 | 0x80);
            }
#line 183 "json_parse.rl"
	{ ps = p; }
#line 189 "json_parse.rl"
	{ {goto st18;} }
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 771 "json_parse.cxx"
	goto st0;
tr39:
#line 139 "json_parse.rl"
	{ u = 0; }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 781 "json_parse.cxx"
	switch( (*p) ) {
		case 68: goto tr48;
		case 100: goto tr50;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr46;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr49;
	} else
		goto tr47;
	goto st0;
tr46:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st22;
tr47:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st22;
tr49:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 811 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr51;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr53;
	} else
		goto tr52;
	goto st0;
tr51:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st23;
tr52:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st23;
tr53:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 837 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr54;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr56;
	} else
		goto tr55;
	goto st0;
tr54:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st24;
tr55:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st24;
tr56:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 863 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr57;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr59;
	} else
		goto tr58;
	goto st0;
tr57:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st25;
tr58:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st25;
tr59:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 889 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr61;
		case 92: goto tr62;
	}
	goto tr60;
tr48:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st26;
tr50:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 907 "json_parse.cxx"
	if ( (*p) < 56 ) {
		if ( 48 <= (*p) && (*p) <= 55 )
			goto tr51;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 68 ) {
			if ( 97 <= (*p) && (*p) <= 100 )
				goto tr65;
		} else if ( (*p) >= 65 )
			goto tr64;
	} else
		goto tr63;
	goto st0;
tr63:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st27;
tr64:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st27;
tr65:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 936 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr66;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr68;
	} else
		goto tr67;
	goto st0;
tr66:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st28;
tr67:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st28;
tr68:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 962 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr69;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr71;
	} else
		goto tr70;
	goto st0;
tr69:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st29;
tr70:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st29;
tr71:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 988 "json_parse.cxx"
	if ( (*p) == 92 )
		goto st30;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 117 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 68: goto tr74;
		case 100: goto tr75;
	}
	goto st0;
tr74:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st32;
tr75:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 1020 "json_parse.cxx"
	if ( (*p) > 70 ) {
		if ( 99 <= (*p) && (*p) <= 102 )
			goto tr77;
	} else if ( (*p) >= 67 )
		goto tr76;
	goto st0;
tr76:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st33;
tr77:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 1039 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr78;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr80;
	} else
		goto tr79;
	goto st0;
tr78:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st34;
tr79:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st34;
tr80:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 1065 "json_parse.cxx"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr81;
	} else if ( (*p) > 70 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto tr83;
	} else
		goto tr82;
	goto st0;
tr81:
#line 133 "json_parse.rl"
	{ u <<= 4; u |= (*p) - '0'; }
	goto st35;
tr82:
#line 134 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'A' + 10; }
	goto st35;
tr83:
#line 135 "json_parse.rl"
	{ u <<= 4; u |= (*p) - 'a' + 10; }
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 1091 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr85;
		case 92: goto tr86;
	}
	goto tr84;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	switch( (*p) ) {
		case 13: goto st36;
		case 32: goto st36;
		case 34: goto tr88;
		case 125: goto tr89;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st36;
	goto st0;
tr88:
#line 193 "json_parse.rl"
	{ ps = p + 1; }
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 1118 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr91;
		case 92: goto tr92;
	}
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	switch( (*p) ) {
		case 34: goto tr93;
		case 92: goto tr94;
	}
	goto st38;
tr91:
#line 194 "json_parse.rl"
	{ lua_pushlstring(L, ps, 0); }
	goto st39;
tr92:
#line 199 "json_parse.rl"
	{ buffer.clear(); { stack.push_back(0); {stack[top++] = 39;goto st18;}} }
	goto st39;
tr93:
#line 196 "json_parse.rl"
	{ lua_pushlstring(L, ps, p - ps); }
	goto st39;
tr94:
#line 197 "json_parse.rl"
	{ size_t n = p - ps; buffer.resize(n); memcpy(buffer.data(), ps, n); { stack.push_back(0); {stack[top++] = 39;goto st18;}} }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 1153 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto st39;
		case 32: goto st39;
		case 58: goto st40;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st39;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 13: goto st40;
		case 32: goto st40;
		case 34: goto tr97;
		case 45: goto tr98;
		case 48: goto tr99;
		case 91: goto tr101;
		case 102: goto st54;
		case 110: goto st58;
		case 116: goto st61;
		case 123: goto tr105;
	}
	if ( (*p) > 10 ) {
		if ( 49 <= (*p) && (*p) <= 57 )
			goto tr100;
	} else if ( (*p) >= 9 )
		goto st40;
	goto st0;
tr97:
#line 193 "json_parse.rl"
	{ ps = p + 1; }
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 1192 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr107;
		case 92: goto tr108;
	}
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 34: goto tr109;
		case 92: goto tr110;
	}
	goto st42;
tr101:
#line 207 "json_parse.rl"
	{ lua_checkstack(L, 2); lua_createtable(L, 8, 0); array_stack.push_back(0); { stack.push_back(0); {stack[top++] = 43;goto st64;}} }
	goto st43;
tr105:
#line 206 "json_parse.rl"
	{ lua_checkstack(L, 3); lua_createtable(L, 0, 8); { stack.push_back(0); {stack[top++] = 43;goto st36;}} }
	goto st43;
tr107:
#line 194 "json_parse.rl"
	{ lua_pushlstring(L, ps, 0); }
	goto st43;
tr108:
#line 199 "json_parse.rl"
	{ buffer.clear(); { stack.push_back(0); {stack[top++] = 43;goto st18;}} }
	goto st43;
tr109:
#line 196 "json_parse.rl"
	{ lua_pushlstring(L, ps, p - ps); }
	goto st43;
tr110:
#line 197 "json_parse.rl"
	{ size_t n = p - ps; buffer.resize(n); memcpy(buffer.data(), ps, n); { stack.push_back(0); {stack[top++] = 43;goto st18;}} }
	goto st43;
tr130:
#line 203 "json_parse.rl"
	{ lua_pushboolean(L, false); }
	goto st43;
tr133:
#line 204 "json_parse.rl"
	{ if (null_index) { lua_pushvalue(L, null_index); } else { lua_pushnil(L); } }
	goto st43;
tr136:
#line 205 "json_parse.rl"
	{ lua_pushboolean(L, true); }
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 1247 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr111;
		case 32: goto tr111;
		case 44: goto tr112;
		case 125: goto tr113;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr111;
	goto st0;
tr111:
#line 212 "json_parse.rl"
	{ lua_rawset(L, -3); }
	goto st44;
tr118:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
#line 212 "json_parse.rl"
	{ lua_rawset(L, -3); }
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 1354 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto st44;
		case 32: goto st44;
		case 44: goto st45;
		case 125: goto tr89;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st44;
	goto st0;
tr112:
#line 212 "json_parse.rl"
	{ lua_rawset(L, -3); }
	goto st45;
tr119:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
#line 212 "json_parse.rl"
	{ lua_rawset(L, -3); }
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 1461 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto st45;
		case 32: goto st45;
		case 34: goto tr88;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st45;
	goto st0;
tr89:
#line 213 "json_parse.rl"
	{ {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st94;
tr113:
#line 212 "json_parse.rl"
	{ lua_rawset(L, -3); }
#line 213 "json_parse.rl"
	{ {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st94;
tr122:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
#line 212 "json_parse.rl"
	{ lua_rawset(L, -3); }
#line 213 "json_parse.rl"
	{ {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 1575 "json_parse.cxx"
	goto st0;
tr98:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
#line 1585 "json_parse.cxx"
	if ( (*p) == 48 )
		goto st47;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto st53;
	goto st0;
tr99:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st47;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
#line 1599 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr118;
		case 32: goto tr118;
		case 44: goto tr119;
		case 46: goto tr120;
		case 69: goto tr121;
		case 101: goto tr121;
		case 125: goto tr122;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr118;
	goto st0;
tr120:
#line 38 "json_parse.rl"
	{ is_int = false; }
	goto st48;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
#line 1620 "json_parse.cxx"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st49;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 13: goto tr118;
		case 32: goto tr118;
		case 44: goto tr119;
		case 69: goto st50;
		case 101: goto st50;
		case 125: goto tr122;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st49;
	} else if ( (*p) >= 9 )
		goto tr118;
	goto st0;
tr121:
#line 39 "json_parse.rl"
	{ is_int = false; }
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 1650 "json_parse.cxx"
	switch( (*p) ) {
		case 43: goto st51;
		case 45: goto st51;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st52;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st52;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 13: goto tr118;
		case 32: goto tr118;
		case 44: goto tr119;
		case 125: goto tr122;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st52;
	} else if ( (*p) >= 9 )
		goto tr118;
	goto st0;
tr100:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
#line 1689 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr118;
		case 32: goto tr118;
		case 44: goto tr119;
		case 46: goto tr120;
		case 69: goto tr121;
		case 101: goto tr121;
		case 125: goto tr122;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st53;
	} else if ( (*p) >= 9 )
		goto tr118;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 97 )
		goto st55;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 108 )
		goto st56;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 115 )
		goto st57;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 101 )
		goto tr130;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 117 )
		goto st59;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 108 )
		goto st60;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 108 )
		goto tr133;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 114 )
		goto st62;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 117 )
		goto st63;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 101 )
		goto tr136;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	switch( (*p) ) {
		case 13: goto st64;
		case 32: goto st64;
		case 34: goto tr138;
		case 45: goto tr139;
		case 48: goto tr140;
		case 91: goto tr142;
		case 93: goto tr143;
		case 102: goto st78;
		case 110: goto st82;
		case 116: goto st85;
		case 123: goto tr147;
	}
	if ( (*p) > 10 ) {
		if ( 49 <= (*p) && (*p) <= 57 )
			goto tr141;
	} else if ( (*p) >= 9 )
		goto st64;
	goto st0;
tr138:
#line 193 "json_parse.rl"
	{ ps = p + 1; }
	goto st65;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
#line 1806 "json_parse.cxx"
	switch( (*p) ) {
		case 34: goto tr149;
		case 92: goto tr150;
	}
	goto st66;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 34: goto tr151;
		case 92: goto tr152;
	}
	goto st66;
tr142:
#line 207 "json_parse.rl"
	{ lua_checkstack(L, 2); lua_createtable(L, 8, 0); array_stack.push_back(0); { stack.push_back(0); {stack[top++] = 67;goto st64;}} }
	goto st67;
tr147:
#line 206 "json_parse.rl"
	{ lua_checkstack(L, 3); lua_createtable(L, 0, 8); { stack.push_back(0); {stack[top++] = 67;goto st36;}} }
	goto st67;
tr149:
#line 194 "json_parse.rl"
	{ lua_pushlstring(L, ps, 0); }
	goto st67;
tr150:
#line 199 "json_parse.rl"
	{ buffer.clear(); { stack.push_back(0); {stack[top++] = 67;goto st18;}} }
	goto st67;
tr151:
#line 196 "json_parse.rl"
	{ lua_pushlstring(L, ps, p - ps); }
	goto st67;
tr152:
#line 197 "json_parse.rl"
	{ size_t n = p - ps; buffer.resize(n); memcpy(buffer.data(), ps, n); { stack.push_back(0); {stack[top++] = 67;goto st18;}} }
	goto st67;
tr172:
#line 203 "json_parse.rl"
	{ lua_pushboolean(L, false); }
	goto st67;
tr175:
#line 204 "json_parse.rl"
	{ if (null_index) { lua_pushvalue(L, null_index); } else { lua_pushnil(L); } }
	goto st67;
tr178:
#line 205 "json_parse.rl"
	{ lua_pushboolean(L, true); }
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 1861 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr153;
		case 32: goto tr153;
		case 44: goto tr154;
		case 93: goto tr155;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr153;
	goto st0;
tr153:
#line 214 "json_parse.rl"
	{ lua_rawseti(L, -2, ++array_stack.back()); }
	goto st68;
tr160:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
#line 214 "json_parse.rl"
	{ lua_rawseti(L, -2, ++array_stack.back()); }
	goto st68;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
#line 1968 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto st68;
		case 32: goto st68;
		case 44: goto st69;
		case 93: goto tr143;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto st68;
	goto st0;
tr154:
#line 214 "json_parse.rl"
	{ lua_rawseti(L, -2, ++array_stack.back()); }
	goto st69;
tr161:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
#line 214 "json_parse.rl"
	{ lua_rawseti(L, -2, ++array_stack.back()); }
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 2075 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto st69;
		case 32: goto st69;
		case 34: goto tr138;
		case 45: goto tr139;
		case 48: goto tr140;
		case 91: goto tr142;
		case 102: goto st78;
		case 110: goto st82;
		case 116: goto st85;
		case 123: goto tr147;
	}
	if ( (*p) > 10 ) {
		if ( 49 <= (*p) && (*p) <= 57 )
			goto tr141;
	} else if ( (*p) >= 9 )
		goto st69;
	goto st0;
tr139:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 2102 "json_parse.cxx"
	if ( (*p) == 48 )
		goto st71;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto st77;
	goto st0;
tr140:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 2116 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr160;
		case 32: goto tr160;
		case 44: goto tr161;
		case 46: goto tr162;
		case 69: goto tr163;
		case 93: goto tr164;
		case 101: goto tr163;
	}
	if ( 9 <= (*p) && (*p) <= 10 )
		goto tr160;
	goto st0;
tr162:
#line 38 "json_parse.rl"
	{ is_int = false; }
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 2137 "json_parse.cxx"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st73;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	switch( (*p) ) {
		case 13: goto tr160;
		case 32: goto tr160;
		case 44: goto tr161;
		case 69: goto st74;
		case 93: goto tr164;
		case 101: goto st74;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st73;
	} else if ( (*p) >= 9 )
		goto tr160;
	goto st0;
tr163:
#line 39 "json_parse.rl"
	{ is_int = false; }
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 2167 "json_parse.cxx"
	switch( (*p) ) {
		case 43: goto st75;
		case 45: goto st75;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st76;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st76;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 13: goto tr160;
		case 32: goto tr160;
		case 44: goto tr161;
		case 93: goto tr164;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st76;
	} else if ( (*p) >= 9 )
		goto tr160;
	goto st0;
tr143:
#line 215 "json_parse.rl"
	{ lua_pushvalue(L, array_index); lua_setmetatable(L, -2); array_stack.pop_back(); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st95;
tr155:
#line 214 "json_parse.rl"
	{ lua_rawseti(L, -2, ++array_stack.back()); }
#line 215 "json_parse.rl"
	{ lua_pushvalue(L, array_index); lua_setmetatable(L, -2); array_stack.pop_back(); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st95;
tr164:
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
#line 214 "json_parse.rl"
	{ lua_rawseti(L, -2, ++array_stack.back()); }
#line 215 "json_parse.rl"
	{ lua_pushvalue(L, array_index); lua_setmetatable(L, -2); array_stack.pop_back(); {cs = stack[--top];{ stack.pop_back(); }goto _again;} }
	goto st95;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
#line 2303 "json_parse.cxx"
	goto st0;
tr141:
#line 41 "json_parse.rl"
	{ ps = p; is_int = true; }
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
#line 2313 "json_parse.cxx"
	switch( (*p) ) {
		case 13: goto tr160;
		case 32: goto tr160;
		case 44: goto tr161;
		case 46: goto tr162;
		case 69: goto tr163;
		case 93: goto tr164;
		case 101: goto tr163;
	}
	if ( (*p) > 10 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st77;
	} else if ( (*p) >= 9 )
		goto tr160;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( (*p) == 97 )
		goto st79;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( (*p) == 108 )
		goto st80;
	goto st0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( (*p) == 115 )
		goto st81;
	goto st0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( (*p) == 101 )
		goto tr172;
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( (*p) == 117 )
		goto st83;
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( (*p) == 108 )
		goto st84;
	goto st0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	if ( (*p) == 108 )
		goto tr175;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( (*p) == 114 )
		goto st86;
	goto st0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( (*p) == 117 )
		goto st87;
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	if ( (*p) == 101 )
		goto tr178;
	goto st0;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
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
	_test_eof93: cs = 93; goto _test_eof; 
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
	_test_eof94: cs = 94; goto _test_eof; 
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
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 89: 
	case 90: 
	case 91: 
	case 92: 
#line 42 "json_parse.rl"
	{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = p - ptr;
              if (n < integer_digs) {
                for (; ptr != p; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != p - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (p != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == p) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = p - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          }
	break;
#line 2590 "json_parse.cxx"
	}
	}

	_out: {}
	}

#line 252 "json_parse.rl"

      if (cs >= 88 && stack.empty()) {
        lua_remove(L, array_index);
        return;
      }

      std::ostringstream out;
      out << "cannot parse json at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
  }

  void initialize_json_parse(lua_State* L) {
    set_field(L, -1, "parse", impl_parse);
  }
}
