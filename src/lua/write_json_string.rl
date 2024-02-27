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
    %%{
      machine json_string_encoder;

      main := |*
        0x08 => { self->write("\\b", 2); };
        0x09 => { self->write("\\t", 2); };
        0x0A => { self->write("\\n", 2); };
        0x0C => { self->write("\\f", 2); };
        0x0D => { self->write("\\r", 2); };

        0x00..0x07 | 0x0B | 0x0E..0x1F => {
          uint8_t v = static_cast<uint8_t>(fc);
          const char data[] = { '\\', 'u', '0', '0', HEX[v >> 4], HEX[v & 0xF] };
          self->write(data, sizeof(data));
        };

        0x22 => { self->write("\\\"", 2); };
        0x2F => { self->write("\\/", 2); };
        0x5C => { self->write("\\\\", 2); };
        0x7F => { self->write("\\u007F", 6); };
        # 0xE2 0x80 0xA8 => { self->write("\\u2028", 6); }; # LINE SEPARATOR
        # 0xE2 0x80 0xA9 => { self->write("\\u2029", 6); }; # PARAGRAPH SEPARATOR

        # Accept not valid UTF-8 characters
        ( 0x20 | 0x21 | 0x23..0x2E | 0x30..0x5B | 0x5D..0x7E
        | 0xC2..0xDF any
        | 0xE0..0xEF any{2} # - 0xE2 0x80 (0xA8 | 0xA9)
        | 0xF0..0xF4 any{3}
        )+ => { self->write(ts, te - ts); };
      *|;

      write data noerror nofinal noentry;
    }%%
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

    %%write init;

    const char* const pb = data.data();
    const char* p = pb;
    const char* const pe = p + data.size();
    const char* const eof = pe;

    self->write('"');
    %%write exec;
    self->write('"');

    if (cs >= %%{ write first_final; }%%) {
      return;
    }

    std::ostringstream out;
    out << "cannot write json string at position " << (p - pb + 1);
    throw BRIGID_RUNTIME_ERROR(out.str());
  }
}
