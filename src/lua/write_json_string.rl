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
    %%{
      machine json_string_encoder;

      escaped
        = 0x00 @{ self->write("\\u0000", 6); }
        | 0x01 @{ self->write("\\u0001", 6); }
        | 0x02 @{ self->write("\\u0002", 6); }
        | 0x03 @{ self->write("\\u0003", 6); }
        | 0x04 @{ self->write("\\u0004", 6); }
        | 0x05 @{ self->write("\\u0005", 6); }
        | 0x06 @{ self->write("\\u0006", 6); }
        | 0x07 @{ self->write("\\u0007", 6); }
        | 0x08 @{ self->write("\\b", 2); }
        | 0x09 @{ self->write("\\t", 2); }
        | 0x0A @{ self->write("\\n", 2); }
        | 0x0B @{ self->write("\\u000B", 6); }
        | 0x0C @{ self->write("\\f", 2); }
        | 0x0D @{ self->write("\\r", 2); }
        | 0x0E @{ self->write("\\u000E", 6); }
        | 0x0F @{ self->write("\\u000F", 6); }
        | 0x10 @{ self->write("\\u0010", 6); }
        | 0x11 @{ self->write("\\u0011", 6); }
        | 0x12 @{ self->write("\\u0012", 6); }
        | 0x13 @{ self->write("\\u0013", 6); }
        | 0x14 @{ self->write("\\u0014", 6); }
        | 0x15 @{ self->write("\\u0015", 6); }
        | 0x16 @{ self->write("\\u0016", 6); }
        | 0x17 @{ self->write("\\u0017", 6); }
        | 0x18 @{ self->write("\\u0018", 6); }
        | 0x19 @{ self->write("\\u0019", 6); }
        | 0x1A @{ self->write("\\u001A", 6); }
        | 0x1B @{ self->write("\\u001B", 6); }
        | 0x1C @{ self->write("\\u001C", 6); }
        | 0x1D @{ self->write("\\u001D", 6); }
        | 0x1E @{ self->write("\\u001E", 6); }
        | 0x1F @{ self->write("\\u001F", 6); }
        | 0x22 @{ self->write("\\\"", 2); }
        | 0x2F @{ self->write("\\/", 2); }
        | 0x5C @{ self->write("\\\\", 2); }
        | 0x7F @{ self->write("\\u007F", 6); }
        ;

      unescaped = any - escaped
        ;

      main :=
        ( escaped+
        | unescaped+ >{ ps = fpc; } %{ self->write(ps, fpc - ps); }
        )**;

      write data noerror nofinal noentry;
    }%%
  }

  void write_json_string(writer_t* self, const char* data, size_t size) {
    int cs = 0;

    %%write init;

    const char* const pb = data;
    const char* p = pb;
    const char* const pe = p + size;
    const char* const eof = pe;
    const char* ps = nullptr;

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
