// vim: syntax=ragel:

// Copyright (c) 2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

namespace brigid {
  namespace {
    %%{
      machine urlencoder;

      unencoded = [^A-Za-z0-9*\-._]+;

      main := unencoded;

      write data noerror nofinal noentry;
    }%%

    template <class T>
    inline void impl_write_urlencoded(T* self, const data_t& data) {
      int cs = 0;

      %%write init;

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();

      %%write exec;
    }
  }
}
