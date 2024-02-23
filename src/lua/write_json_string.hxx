
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
static const int json_string_encoder_start = 9;


#line 57 "write_json_string.rl"


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

#line 69 "write_json_string.rl"

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      self->write('"');
      
#line 51 "write_json_string.hxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr0:
#line 47 "write_json_string.rl"
	{ self->write(p - 1, 2); }
	goto st9;
tr2:
#line 51 "write_json_string.rl"
	{ self->write(p - 2, 3); }
	goto st9;
tr4:
#line 49 "write_json_string.rl"
	{ self->write("\\u2028", 6); }
	goto st9;
tr5:
#line 50 "write_json_string.rl"
	{ self->write("\\u2029", 6); }
	goto st9;
tr8:
#line 53 "write_json_string.rl"
	{ self->write(p - 3, 4); }
	goto st9;
tr14:
#line 35 "write_json_string.rl"
	{
            uint8_t v = static_cast<uint8_t>((*p));
            const char data[] = { '\\', 'u', '0', '0', HEX[v >> 4], HEX[v & 0xF] };
            self->write(data, sizeof(data));
          }
	goto st9;
tr15:
#line 30 "write_json_string.rl"
	{ self->write("\\b", 2); }
	goto st9;
tr16:
#line 31 "write_json_string.rl"
	{ self->write("\\t", 2); }
	goto st9;
tr17:
#line 32 "write_json_string.rl"
	{ self->write("\\n", 2); }
	goto st9;
tr18:
#line 33 "write_json_string.rl"
	{ self->write("\\f", 2); }
	goto st9;
tr19:
#line 34 "write_json_string.rl"
	{ self->write("\\r", 2); }
	goto st9;
tr20:
#line 45 "write_json_string.rl"
	{ self->write((*p)); }
	goto st9;
tr21:
#line 41 "write_json_string.rl"
	{ self->write("\\\"", 2); }
	goto st9;
tr22:
#line 42 "write_json_string.rl"
	{ self->write("\\/", 2); }
	goto st9;
tr23:
#line 43 "write_json_string.rl"
	{ self->write("\\\\", 2); }
	goto st9;
tr24:
#line 44 "write_json_string.rl"
	{ self->write("\\u007F", 6); }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 129 "write_json_string.hxx"
	switch( (*p) ) {
		case -30: goto st4;
		case 8: goto tr15;
		case 9: goto tr16;
		case 10: goto tr17;
		case 12: goto tr18;
		case 13: goto tr19;
		case 34: goto tr21;
		case 47: goto tr22;
		case 92: goto tr23;
		case 127: goto tr24;
	}
	if ( (*p) < -32 ) {
		if ( (*p) > -63 ) {
			if ( -62 <= (*p) && (*p) <= -33 )
				goto st1;
		} else
			goto st0;
	} else if ( (*p) > -17 ) {
		if ( (*p) < -11 ) {
			if ( -16 <= (*p) && (*p) <= -12 )
				goto st6;
		} else if ( (*p) > -1 ) {
			if ( 0 <= (*p) && (*p) <= 31 )
				goto tr14;
		} else
			goto st0;
	} else
		goto st2;
	goto tr20;
st0:
cs = 0;
	goto _out;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	goto tr0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	goto tr2;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == -128 )
		goto st5;
	goto st3;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case -88: goto tr4;
		case -87: goto tr5;
	}
	goto tr2;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	goto tr8;
	}
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 76 "write_json_string.rl"
      self->write('"');

      if (cs >= 9) {
        return;
      }

      std::ostringstream out;
      out << "cannot encode json string at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }
  }
}

#endif
