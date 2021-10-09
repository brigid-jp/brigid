// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <stdlib.h>
#include <iostream>
#include <utility>
#include <vector>

namespace brigid {
  namespace {
    %%{
      machine json_parser;

      prepush { stack.push_back(0); }
      postpop { stack.pop_back(); }

      ws = [ \t\n\r]*;

      number
        = ( ("-" @{ is_minus = true; })?
            (0 | [1-9] digit*) ${ v *= 10; v += fc - '0'; }
            ("." digit+ @{ is_integer = false; })?
            ([eE] [+\-]? digit+ @{ is_integer = false; })?
          ) >{
              ps = p;
              is_minus = false;
              is_integer = true;
              v = 0;
            }
            %{
              if (is_integer) {
                lua_pushinteger(L, is_minus ? -v : v);
              } else {
                // 入力文字列が\0終端していれば、そのまま渡しても安全である
                // なぜなら、numberの後に出現する可能性があるのは、
                // whitespace
                // ], },
                // \0
                // くらいだから
                // TODO 厳密に保証すること
                char* end = nullptr;
                double u = strtod(ps, &end);
                // TODO error check
                lua_pushnumber(L, u);
              }
            };

      utf8_tail = 0x80..0xBF;
      utf8_char
        = 0x00..0x7F
        | 0xC2..0xDF utf8_tail
        | 0xE0 0xA0..0xBF utf8_tail | 0xE1..0xEC utf8_tail{2}
        | 0xED 0x80..0x9F utf8_tail | 0xEE..0xEF utf8_tail{2}
        | 0xF0 0x90..0xBF utf8_tail{2} | 0xF1..0xF3 utf8_tail{3}
        | 0xF4 0x80..0x8F utf8_tail{2};
      unescaped = utf8_char - [\\\"];

      string
        = "\"" @{ buffer.clear(); }
          ( "\\\"" @{ buffer.push_back('"'); }
          | "\\\\" @{ buffer.push_back('\\'); }
          | "\\/" @{ buffer.push_back('/'); }
          | "\\b" @{ buffer.push_back('\b'); }
          | "\\f" @{ buffer.push_back('\f'); }
          | "\\n" @{ buffer.push_back('\n'); }
          | "\\r" @{ buffer.push_back('\r'); }
          | "\\t" @{ buffer.push_back('\t'); }
          # TODO \u
          | unescaped ${ buffer.push_back(fc); }
          )*
          "\"" @{
            lua_pushlstring(L, buffer.data(), buffer.size());
          };

      value
        = "false" @{ lua_pushboolean(L, false); }
        | "null" @{ lua_pushnil(L); }
        | "true" @{ lua_pushboolean(L, true); }
        | "{" @{ lua_newtable(L); fcall object; }
        | "[" @{ lua_newtable(L); n = 0; fcall array; }
        | number
        | string;

      member = ws string ws ":" ws value %{
        std::cout << "settable\n";
        lua_settable(L, -3);
      };
      object := (member (ws "," member)*)? ws "}" @{ fret; };

      element = ws value @{ lua_seti(L, -2, ++n); };
      array := (element (ws "," element)*)? ws "]" @{ fret; };
      main := ws value ws;

      write data noerror nofinal noentry;
    }%%


    void impl_decode(lua_State* L) {
      data_t data = check_data(L, 1);

      int cs;
      int top;
      %%write init;

      const char* p = data.data();
      const char* pe = p + data.size();
      const char* eof = pe;
      std::vector<int> stack; // TODO reserve

      const char* ps;
      lua_Integer n;
      lua_Integer v;
      std::vector<char> buffer; // TODO reserve
      bool is_minus;
      bool is_integer;

      %%write exec;

      if (cs >= %%{ write first_final; }%%) {
        // check stack size == 1
      } else {
        // throw error
      }
    }
  }

  void initialize_json(lua_State* L) {
    lua_newtable(L);
    {
      set_field(L, -1, "decode", impl_decode);
    }
    set_field(L, -2, "json");
  }
}
