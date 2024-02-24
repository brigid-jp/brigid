// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_WRITE_URLENCODED_HPP
#define BRIGID_WRITE_URLENCODED_HPP

#include "data.hpp"
#include "error.hpp"

#include <stdint.h>

namespace brigid {
  namespace {
    %%{
      machine urlencoder;

      unencoded_set = alnum | "*" | "-" | "." | "_";

      main :=
        ( " " @{ self->write('+'); }
        | unencoded_set @{ self->write(fc); }
        | (any - " " - unencoded_set) @{
            uint8_t v = static_cast<uint8_t>(fc);
            const char data[] = { '%', HEX[v >> 4], HEX[v & 0xF] };
            self->write(data, sizeof(data));
          }
        )*;

      write data noerror nofinal noentry;
    }%%

    template <class T>
    inline void impl_write_urlencoded(T* self, const data_t& data) {
      static const char HEX[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
      };

      int cs = 0;

      %%write init;

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      %%write exec;

      if (cs >= %%{ write first_final; }%%) {
        return;
      }

      std::ostringstream out;
      out << "cannot write urlencoded at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }
  }
}

#endif
