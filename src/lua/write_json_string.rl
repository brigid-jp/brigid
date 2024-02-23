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
    %%{
      machine json_string_encoder;

      # Accept not valid UTF-8 characters
      utf8_1 = 0x00..0x7F;
      utf8_2 = 0xC2..0xDF any;
      utf8_3 = 0xE0..0xEF any{2};
      utf8_4 = 0xF0..0xF4 any{3};

      LINE_SEPARATOR = 0xE2 0x80 0xA8;
      PARA_SEPARATOR = 0xE2 0x80 0xA9;

      main :=
        ( 0x08 @{ self->write("\\b", 2); }
        | 0x09 @{ self->write("\\t", 2); }
        | 0x0A @{ self->write("\\n", 2); }
        | 0x0C @{ self->write("\\f", 2); }
        | 0x0D @{ self->write("\\r", 2); }
        | (0x00..0x07 | 0x0B | 0x0E..0x1F) @{
            uint8_t v = static_cast<uint8_t>(fc);
            const char data[] = { '\\', 'u', '0', '0', HEX[v >> 4], HEX[v & 0xF] };
            self->write(data, sizeof(data));
          }

        | 0x22 @{ self->write("\\\"", 2); }
        | 0x2F @{ self->write("\\/", 2); }
        | 0x5C @{ self->write("\\\\", 2); }
        | 0x7F @{ self->write("\\u007F", 6); }
        | (0x20 | 0x21 | 0x23..0x2E | 0x30..0x5B | 0x5D..0x7E) @{ self->write(fc); }

        | utf8_2 @{ self->write(fpc - 1, 2); }

        | LINE_SEPARATOR @{ self->write("\\u2028", 6); }
        | PARA_SEPARATOR @{ self->write("\\u2029", 6); }
        | (utf8_3 - LINE_SEPARATOR - PARA_SEPARATOR) @{ self->write(fpc - 2, 3); }

        | utf8_4 @{ self->write(fpc - 3, 4); }
        )*;

      write data noerror nofinal noentry;
    }%%

    template <class T>
    inline void impl_write_json_string(T* self, const data_t& data) {
      static const char HEX[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F',
      };

      int cs = 0;

      %%write init;

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      self->write('"');
      %%write exec;
      self->write('"');

      if (cs >= %%{ write first_final; }%%) {
        return;
      }

      std::ostringstream out;
      out << "cannot encode json string at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }
  }
}

#endif
