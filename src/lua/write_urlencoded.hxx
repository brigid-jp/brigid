
#line 1 "write_urlencoded.rl"
// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "data.hpp"
#include "error.hpp"

#include <stdint.h>

namespace brigid {
  namespace {
    
#line 18 "write_urlencoded.hxx"
static const int urlencoder_start = 0;


#line 28 "write_urlencoded.rl"


    template <class T>
    inline void impl_write_urlencoded(T* self, const data_t& data) {
      static const char HEX[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F',
      };

      int cs = 0;

      
#line 35 "write_urlencoded.hxx"
	{
	cs = urlencoder_start;
	}

#line 40 "write_urlencoded.rl"

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      
#line 47 "write_urlencoded.hxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr0:
#line 20 "write_urlencoded.rl"
	{
            uint8_t v = static_cast<uint8_t>((*p));
            const char data[] = { '%', HEX[v >> 4], HEX[v & 0xF] };
            self->write(data, sizeof(data));
          }
	goto st0;
tr1:
#line 18 "write_urlencoded.rl"
	{ self->write('+'); }
	goto st0;
tr2:
#line 19 "write_urlencoded.rl"
	{ self->write((*p)); }
	goto st0;
st0:
	if ( ++p == pe )
		goto _test_eof0;
case 0:
#line 73 "write_urlencoded.hxx"
	switch( (*p) ) {
		case 32: goto tr1;
		case 42: goto tr2;
		case 95: goto tr2;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr2;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr2;
		} else if ( (*p) >= 65 )
			goto tr2;
	} else
		goto tr2;
	goto tr0;
	}
	_test_eof0: cs = 0; goto _test_eof; 

	_test_eof: {}
	}

#line 46 "write_urlencoded.rl"

      if (cs >= 0) {
        return;
      }

      std::ostringstream out;
      out << "cannot percent-encode at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }
  }
}
