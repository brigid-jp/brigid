
#line 1 "write_urlencoded.rl"
// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

namespace brigid {
  namespace {
    
#line 13 "write_urlencoded.hxx"
static const int urlencoder_start = 1;


#line 17 "write_urlencoded.rl"


    template <class T>
    inline void impl_write_urlencoded(T* self, const data_t& data) {
      int cs = 0;

      
#line 25 "write_urlencoded.hxx"
	{
	cs = urlencoder_start;
	}

#line 24 "write_urlencoded.rl"

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      
#line 37 "write_urlencoded.hxx"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 42: goto st0;
		case 95: goto st0;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st0;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st0;
		} else if ( (*p) >= 65 )
			goto st0;
	} else
		goto st0;
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 42: goto st0;
		case 95: goto st0;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st0;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st0;
		} else if ( (*p) >= 65 )
			goto st0;
	} else
		goto st0;
	goto st2;
st0:
cs = 0;
	goto _out;
	}
	_test_eof2: cs = 2; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 30 "write_urlencoded.rl"
    }
  }
}
