// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <stdint.h>

namespace brigid {
  namespace {
    %%{
      machine urlencoder;

      main :=
        ( [A-Za-z0-9*\-._] @{ self->write(fc); }
        | ' ' @{ self->write('+'); }
        | [^A-Za-z0-9*\-._ ] @{
            uint8_t v = static_cast<uint8_t>(fc);
            const char data[] = { '%', HEX[v >> 4], HEX[v & 0x0F] };
            self->write(data, sizeof(data));
          }
        )*;

      write data noerror nofinal noentry;
    }%%

    template <class T>
    inline void impl_write_urlencoded(T* self, const data_t& data) {
      static const char HEX[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F',
      };

      int cs = 0;

      %%write init;

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      %%write exec;
    }
  }
}
