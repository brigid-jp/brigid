
#line 1 "write_json_string.rl"
// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "data.hpp"
#include "error.hpp"
#include "writer.hpp"

#include <stdint.h>
#include <sstream>

namespace brigid {
  namespace {
    
#line 20 "write_json_string.cxx"
static const int json_string_encoder_start = 8;


#line 109 "write_json_string.rl"

  }

  void write_json_string(writer_t* self, const data_t& data) {
    static const char HEX[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    };

    if (data.size() == 0) {
      self->write("\"\"", 2);
      return;
    }

    int cs = 0;
    int act = 0;
    const char* ts = nullptr;
    const char* te = nullptr;

    
#line 44 "write_json_string.cxx"
	{
	cs = json_string_encoder_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 128 "write_json_string.rl"

    const char* const pb = data.data();
    const char* p = pb;
    const char* const pe = p + data.size();
    const char* const eof = pe;

    // const char* ps = nullptr;

    self->write('"');
    
#line 63 "write_json_string.cxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr0:
#line 1 "NONE"
	{	switch( act ) {
	case 0:
	{{goto st0;}}
	break;
	case 13:
	{{p = ((te))-1;} self->write(ts, te - ts); }
	break;
	}
	}
	goto st8;
tr3:
#line 69 "write_json_string.rl"
	{{p = ((te))-1;}{ self->write(ts, te - ts); }}
	goto st8;
tr7:
#line 62 "write_json_string.rl"
	{te = p+1;{ self->write("\\u2028", 6); }}
	goto st8;
tr8:
#line 63 "write_json_string.rl"
	{te = p+1;{ self->write("\\u2029", 6); }}
	goto st8;
tr12:
#line 52 "write_json_string.rl"
	{te = p+1;{
          uint8_t v = static_cast<uint8_t>((*p));
          const char data[] = { '\\', 'u', '0', '0', HEX[v >> 4], HEX[v & 0xF] };
          self->write(data, sizeof(data));
        }}
	goto st8;
tr13:
#line 46 "write_json_string.rl"
	{te = p+1;{ self->write("\\b", 2); }}
	goto st8;
tr14:
#line 47 "write_json_string.rl"
	{te = p+1;{ self->write("\\t", 2); }}
	goto st8;
tr15:
#line 48 "write_json_string.rl"
	{te = p+1;{ self->write("\\n", 2); }}
	goto st8;
tr16:
#line 49 "write_json_string.rl"
	{te = p+1;{ self->write("\\f", 2); }}
	goto st8;
tr17:
#line 50 "write_json_string.rl"
	{te = p+1;{ self->write("\\r", 2); }}
	goto st8;
tr18:
#line 58 "write_json_string.rl"
	{te = p+1;{ self->write("\\\"", 2); }}
	goto st8;
tr19:
#line 59 "write_json_string.rl"
	{te = p+1;{ self->write("\\/", 2); }}
	goto st8;
tr20:
#line 60 "write_json_string.rl"
	{te = p+1;{ self->write("\\\\", 2); }}
	goto st8;
tr21:
#line 61 "write_json_string.rl"
	{te = p+1;{ self->write("\\u007F", 6); }}
	goto st8;
tr22:
#line 69 "write_json_string.rl"
	{te = p;p--;{ self->write(ts, te - ts); }}
	goto st8;
st8:
#line 1 "NONE"
	{ts = 0;}
#line 1 "NONE"
	{act = 0;}
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 1 "NONE"
	{ts = p;}
#line 151 "write_json_string.cxx"
	switch( (*p) ) {
		case -30: goto st6;
		case 8: goto tr13;
		case 9: goto tr14;
		case 10: goto tr15;
		case 12: goto tr16;
		case 13: goto tr17;
		case 34: goto tr18;
		case 47: goto tr19;
		case 92: goto tr20;
		case 127: goto tr21;
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
				goto st5;
		} else if ( (*p) > -1 ) {
			if ( 0 <= (*p) && (*p) <= 31 )
				goto tr12;
		} else
			goto st0;
	} else
		goto st2;
	goto tr1;
st0:
cs = 0;
	goto _out;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	goto tr1;
tr1:
#line 1 "NONE"
	{te = p+1;}
#line 69 "write_json_string.rl"
	{act = 13;}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 200 "write_json_string.cxx"
	if ( (*p) == -30 )
		goto st3;
	if ( (*p) < 32 ) {
		if ( (*p) < -32 ) {
			if ( -62 <= (*p) && (*p) <= -33 )
				goto st1;
		} else if ( (*p) > -17 ) {
			if ( -16 <= (*p) && (*p) <= -12 )
				goto st5;
		} else
			goto st2;
	} else if ( (*p) > 33 ) {
		if ( (*p) < 48 ) {
			if ( 35 <= (*p) && (*p) <= 46 )
				goto tr1;
		} else if ( (*p) > 91 ) {
			if ( 93 <= (*p) && (*p) <= 126 )
				goto tr1;
		} else
			goto tr1;
	} else
		goto tr1;
	goto tr22;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	goto st1;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == -128 )
		goto st4;
	goto st1;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( -88 <= (*p) && (*p) <= -87 )
		goto tr3;
	goto tr1;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	goto st2;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == -128 )
		goto st7;
	goto st1;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case -88: goto tr7;
		case -87: goto tr8;
	}
	goto tr1;
	}
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 1: goto tr0;
	case 9: goto tr22;
	case 2: goto tr0;
	case 3: goto tr3;
	case 4: goto tr3;
	case 5: goto tr0;
	}
	}

	_out: {}
	}

#line 138 "write_json_string.rl"
    self->write('"');

    if (cs >= 8) {
      return;
    }

    std::ostringstream out;
    out << "cannot write json string at position " << (p - pb + 1);
    throw BRIGID_RUNTIME_ERROR(out.str());
  }
}
