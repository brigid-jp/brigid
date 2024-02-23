
#line 1 "write_json_string.rl"
// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_WRITE_JSON_STRING_HXX
#define BRIGID_WRITE_JSON_STRING_HXX

#include "data.hpp"
#include "error.hpp"

#include <stdint.h>

namespace brigid {
  namespace {
    
#line 21 "write_json_string.hxx"
static const int json_string_encoder_start = 3;


#line 40 "write_json_string.rl"


    template <class T>
    inline void impl_write_json_string(T* self, const data_t& data) {
      static const char HEX[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F',
      };

      int cs = 0;

      
#line 38 "write_json_string.hxx"
	{
	cs = json_string_encoder_start;
	}

#line 52 "write_json_string.rl"

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      
#line 50 "write_json_string.hxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr2:
#line 35 "write_json_string.rl"
	{ self->write("\\u2028", 6); }
	goto st3;
tr3:
#line 36 "write_json_string.rl"
	{ self->write("\\u2029", 6); }
	goto st3;
tr5:
#line 29 "write_json_string.rl"
	{
            uint8_t v = static_cast<uint8_t>((*p));
            const char data[] = { '\\', '0', '0', HEX[v >> 4], HEX[v & 0xF] };
            self->write(data, sizeof(data));
          }
	goto st3;
tr6:
#line 24 "write_json_string.rl"
	{ self->write("\\b", 2); }
	goto st3;
tr7:
#line 28 "write_json_string.rl"
	{ self->write("\\t", 2); }
	goto st3;
tr8:
#line 26 "write_json_string.rl"
	{ self->write("\\n", 2); }
	goto st3;
tr9:
#line 25 "write_json_string.rl"
	{ self->write("\\f", 2); }
	goto st3;
tr10:
#line 27 "write_json_string.rl"
	{ self->write("\\r", 2); }
	goto st3;
tr11:
#line 21 "write_json_string.rl"
	{ self->write("\\\"", 2); }
	goto st3;
tr12:
#line 23 "write_json_string.rl"
	{ self->write("\\/", 2); }
	goto st3;
tr13:
#line 22 "write_json_string.rl"
	{ self->write("\\\\", 2); }
	goto st3;
tr14:
#line 34 "write_json_string.rl"
	{ self->write("\\u007F", 6); }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 112 "write_json_string.hxx"
	switch( (*p) ) {
		case -30: goto st1;
		case 8: goto tr6;
		case 9: goto tr7;
		case 10: goto tr8;
		case 12: goto tr9;
		case 13: goto tr10;
		case 34: goto tr11;
		case 47: goto tr12;
		case 92: goto tr13;
		case 127: goto tr14;
	}
	if ( 0 <= (*p) && (*p) <= 31 )
		goto tr5;
	goto st0;
st0:
cs = 0;
	goto _out;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	if ( (*p) == -128 )
		goto st2;
	goto st0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case -88: goto tr2;
		case -87: goto tr3;
	}
	goto st0;
	}
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 58 "write_json_string.rl"

      if (cs >= 3) {
        return;
      }

      std::ostringstream out;
      out << "cannot encode json string at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }
  }
}

#endif
