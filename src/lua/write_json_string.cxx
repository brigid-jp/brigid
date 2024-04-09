
#line 1 "write_json_string.rl"
// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "error.hpp"
#include "writer.hpp"

#include <stddef.h>
#include <sstream>

namespace brigid {
  namespace {
    
#line 19 "write_json_string.cxx"
static const int json_string_encoder_start = 0;


#line 65 "write_json_string.rl"

  }

  void write_json_string(writer_t* self, const char* pb, size_t size) {
    int cs = 0;

    
#line 31 "write_json_string.cxx"
	{
	cs = json_string_encoder_start;
	}

#line 72 "write_json_string.rl"

    const char* p = pb;
    const char* const pe = p + size;
    const char* const eof = pe;
    const char* ps = nullptr;

    self->write('"');
    
#line 45 "write_json_string.cxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr1:
#line 19 "write_json_string.rl"
	{ self->write("\\u0000", 6); }
	goto st0;
tr2:
#line 20 "write_json_string.rl"
	{ self->write("\\u0001", 6); }
	goto st0;
tr3:
#line 21 "write_json_string.rl"
	{ self->write("\\u0002", 6); }
	goto st0;
tr4:
#line 22 "write_json_string.rl"
	{ self->write("\\u0003", 6); }
	goto st0;
tr5:
#line 23 "write_json_string.rl"
	{ self->write("\\u0004", 6); }
	goto st0;
tr6:
#line 24 "write_json_string.rl"
	{ self->write("\\u0005", 6); }
	goto st0;
tr7:
#line 25 "write_json_string.rl"
	{ self->write("\\u0006", 6); }
	goto st0;
tr8:
#line 26 "write_json_string.rl"
	{ self->write("\\u0007", 6); }
	goto st0;
tr9:
#line 27 "write_json_string.rl"
	{ self->write("\\b", 2); }
	goto st0;
tr10:
#line 28 "write_json_string.rl"
	{ self->write("\\t", 2); }
	goto st0;
tr11:
#line 29 "write_json_string.rl"
	{ self->write("\\n", 2); }
	goto st0;
tr12:
#line 30 "write_json_string.rl"
	{ self->write("\\u000B", 6); }
	goto st0;
tr13:
#line 31 "write_json_string.rl"
	{ self->write("\\f", 2); }
	goto st0;
tr14:
#line 32 "write_json_string.rl"
	{ self->write("\\r", 2); }
	goto st0;
tr15:
#line 33 "write_json_string.rl"
	{ self->write("\\u000E", 6); }
	goto st0;
tr16:
#line 34 "write_json_string.rl"
	{ self->write("\\u000F", 6); }
	goto st0;
tr17:
#line 35 "write_json_string.rl"
	{ self->write("\\u0010", 6); }
	goto st0;
tr18:
#line 36 "write_json_string.rl"
	{ self->write("\\u0011", 6); }
	goto st0;
tr19:
#line 37 "write_json_string.rl"
	{ self->write("\\u0012", 6); }
	goto st0;
tr20:
#line 38 "write_json_string.rl"
	{ self->write("\\u0013", 6); }
	goto st0;
tr21:
#line 39 "write_json_string.rl"
	{ self->write("\\u0014", 6); }
	goto st0;
tr22:
#line 40 "write_json_string.rl"
	{ self->write("\\u0015", 6); }
	goto st0;
tr23:
#line 41 "write_json_string.rl"
	{ self->write("\\u0016", 6); }
	goto st0;
tr24:
#line 42 "write_json_string.rl"
	{ self->write("\\u0017", 6); }
	goto st0;
tr25:
#line 43 "write_json_string.rl"
	{ self->write("\\u0018", 6); }
	goto st0;
tr26:
#line 44 "write_json_string.rl"
	{ self->write("\\u0019", 6); }
	goto st0;
tr27:
#line 45 "write_json_string.rl"
	{ self->write("\\u001A", 6); }
	goto st0;
tr28:
#line 46 "write_json_string.rl"
	{ self->write("\\u001B", 6); }
	goto st0;
tr29:
#line 47 "write_json_string.rl"
	{ self->write("\\u001C", 6); }
	goto st0;
tr30:
#line 48 "write_json_string.rl"
	{ self->write("\\u001D", 6); }
	goto st0;
tr31:
#line 49 "write_json_string.rl"
	{ self->write("\\u001E", 6); }
	goto st0;
tr32:
#line 50 "write_json_string.rl"
	{ self->write("\\u001F", 6); }
	goto st0;
tr33:
#line 51 "write_json_string.rl"
	{ self->write("\\\"", 2); }
	goto st0;
tr34:
#line 52 "write_json_string.rl"
	{ self->write("\\\\", 2); }
	goto st0;
tr35:
#line 53 "write_json_string.rl"
	{ self->write("\\u007F", 6); }
	goto st0;
tr37:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 19 "write_json_string.rl"
	{ self->write("\\u0000", 6); }
	goto st0;
tr38:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 20 "write_json_string.rl"
	{ self->write("\\u0001", 6); }
	goto st0;
tr39:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 21 "write_json_string.rl"
	{ self->write("\\u0002", 6); }
	goto st0;
tr40:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 22 "write_json_string.rl"
	{ self->write("\\u0003", 6); }
	goto st0;
tr41:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 23 "write_json_string.rl"
	{ self->write("\\u0004", 6); }
	goto st0;
tr42:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 24 "write_json_string.rl"
	{ self->write("\\u0005", 6); }
	goto st0;
tr43:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 25 "write_json_string.rl"
	{ self->write("\\u0006", 6); }
	goto st0;
tr44:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 26 "write_json_string.rl"
	{ self->write("\\u0007", 6); }
	goto st0;
tr45:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 27 "write_json_string.rl"
	{ self->write("\\b", 2); }
	goto st0;
tr46:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 28 "write_json_string.rl"
	{ self->write("\\t", 2); }
	goto st0;
tr47:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 29 "write_json_string.rl"
	{ self->write("\\n", 2); }
	goto st0;
tr48:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 30 "write_json_string.rl"
	{ self->write("\\u000B", 6); }
	goto st0;
tr49:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 31 "write_json_string.rl"
	{ self->write("\\f", 2); }
	goto st0;
tr50:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 32 "write_json_string.rl"
	{ self->write("\\r", 2); }
	goto st0;
tr51:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 33 "write_json_string.rl"
	{ self->write("\\u000E", 6); }
	goto st0;
tr52:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 34 "write_json_string.rl"
	{ self->write("\\u000F", 6); }
	goto st0;
tr53:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 35 "write_json_string.rl"
	{ self->write("\\u0010", 6); }
	goto st0;
tr54:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 36 "write_json_string.rl"
	{ self->write("\\u0011", 6); }
	goto st0;
tr55:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 37 "write_json_string.rl"
	{ self->write("\\u0012", 6); }
	goto st0;
tr56:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 38 "write_json_string.rl"
	{ self->write("\\u0013", 6); }
	goto st0;
tr57:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 39 "write_json_string.rl"
	{ self->write("\\u0014", 6); }
	goto st0;
tr58:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 40 "write_json_string.rl"
	{ self->write("\\u0015", 6); }
	goto st0;
tr59:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 41 "write_json_string.rl"
	{ self->write("\\u0016", 6); }
	goto st0;
tr60:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 42 "write_json_string.rl"
	{ self->write("\\u0017", 6); }
	goto st0;
tr61:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 43 "write_json_string.rl"
	{ self->write("\\u0018", 6); }
	goto st0;
tr62:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 44 "write_json_string.rl"
	{ self->write("\\u0019", 6); }
	goto st0;
tr63:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 45 "write_json_string.rl"
	{ self->write("\\u001A", 6); }
	goto st0;
tr64:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 46 "write_json_string.rl"
	{ self->write("\\u001B", 6); }
	goto st0;
tr65:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 47 "write_json_string.rl"
	{ self->write("\\u001C", 6); }
	goto st0;
tr66:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 48 "write_json_string.rl"
	{ self->write("\\u001D", 6); }
	goto st0;
tr67:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 49 "write_json_string.rl"
	{ self->write("\\u001E", 6); }
	goto st0;
tr68:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 50 "write_json_string.rl"
	{ self->write("\\u001F", 6); }
	goto st0;
tr69:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 51 "write_json_string.rl"
	{ self->write("\\\"", 2); }
	goto st0;
tr70:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 52 "write_json_string.rl"
	{ self->write("\\\\", 2); }
	goto st0;
tr71:
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
#line 53 "write_json_string.rl"
	{ self->write("\\u007F", 6); }
	goto st0;
st0:
	if ( ++p == pe )
		goto _test_eof0;
case 0:
#line 405 "write_json_string.cxx"
	switch( (*p) ) {
		case 0: goto tr1;
		case 1: goto tr2;
		case 2: goto tr3;
		case 3: goto tr4;
		case 4: goto tr5;
		case 5: goto tr6;
		case 6: goto tr7;
		case 7: goto tr8;
		case 8: goto tr9;
		case 9: goto tr10;
		case 10: goto tr11;
		case 11: goto tr12;
		case 12: goto tr13;
		case 13: goto tr14;
		case 14: goto tr15;
		case 15: goto tr16;
		case 16: goto tr17;
		case 17: goto tr18;
		case 18: goto tr19;
		case 19: goto tr20;
		case 20: goto tr21;
		case 21: goto tr22;
		case 22: goto tr23;
		case 23: goto tr24;
		case 24: goto tr25;
		case 25: goto tr26;
		case 26: goto tr27;
		case 27: goto tr28;
		case 28: goto tr29;
		case 29: goto tr30;
		case 30: goto tr31;
		case 31: goto tr32;
		case 34: goto tr33;
		case 92: goto tr34;
		case 127: goto tr35;
	}
	goto tr0;
tr0:
#line 61 "write_json_string.rl"
	{ ps = p; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 452 "write_json_string.cxx"
	switch( (*p) ) {
		case 0: goto tr37;
		case 1: goto tr38;
		case 2: goto tr39;
		case 3: goto tr40;
		case 4: goto tr41;
		case 5: goto tr42;
		case 6: goto tr43;
		case 7: goto tr44;
		case 8: goto tr45;
		case 9: goto tr46;
		case 10: goto tr47;
		case 11: goto tr48;
		case 12: goto tr49;
		case 13: goto tr50;
		case 14: goto tr51;
		case 15: goto tr52;
		case 16: goto tr53;
		case 17: goto tr54;
		case 18: goto tr55;
		case 19: goto tr56;
		case 20: goto tr57;
		case 21: goto tr58;
		case 22: goto tr59;
		case 23: goto tr60;
		case 24: goto tr61;
		case 25: goto tr62;
		case 26: goto tr63;
		case 27: goto tr64;
		case 28: goto tr65;
		case 29: goto tr66;
		case 30: goto tr67;
		case 31: goto tr68;
		case 34: goto tr69;
		case 92: goto tr70;
		case 127: goto tr71;
	}
	goto st1;
	}
	_test_eof0: cs = 0; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 1: 
#line 61 "write_json_string.rl"
	{ self->write(ps, p - ps); }
	break;
#line 503 "write_json_string.cxx"
	}
	}

	}

#line 80 "write_json_string.rl"
    self->write('"');

    if (cs >= 0) {
      return;
    }

    std::ostringstream out;
    out << "cannot write json string at position " << (p - pb + 1);
    throw BRIGID_RUNTIME_ERROR(out.str());
  }
}
