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
        = ( ("-" @{ is_neg = true; })?
            (0 | [1-9] digit*) ${ v *= 10; v += fc - '0'; }
            ("." digit+ @{ is_int = false; })?
            ([eE] [+\-]? digit+ @{ is_int = false; })?
          ) >{
              ps = p;
              is_neg = false;
              is_int = true;
              v = 0;
            }
            %{
              if (is_int) {
                // TODO 整数の範囲内か確かめる: Lua 5.3以降のコードを確認
                // TODO LuaJITは？
                lua_pushinteger(L, is_neg ? -v : v);
              } else {
                // 入力文字列が\0もしくは他の区切り文字で終端していることは保証されない
                size_t size = p - ps;
                buffer.resize(size + 1);
                memcpy(buffer.data(), ps, size);
                buffer[size] = '\0';
                char* end = nullptr;
                double u = strtod(buffer.data(), &end);
                // TODO locale check
                // TODO error check
                lua_pushnumber(L, u);
              }
            };

      unescaped = (0x20 | 0x21 | 0x23..0x5B | 0x5D..0x7F | 0x80..0xFF);

      hex_quad =
        ( [0-9] @{ u <<= 4; u |= fc - '0'; }
        | [A-F] @{ u <<= 4; u |= fc - 'A' + 10; }
        | [a-f] @{ u <<= 4; u |= fc - 'a' + 10; }
        ){4};

      unicode_escape_sequence =
        "\\u" @{ u = 0; }
        ( (hex_quad & (xdigit{4} - (/D[89A-F]/i xdigit{2})))
            %{
              if (u <= 0x007F) {
                buffer.push_back(u);
              } else if (u <= 0x07FF) {
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xC0);
                buffer.push_back(u2 | 0x80);
              } else {
                uint8_t u3 = u & 0x3F; u >>= 6;
                uint8_t u2 = u & 0x3F; u >>= 6;
                buffer.push_back(u  | 0xE0);
                buffer.push_back(u2 | 0x80);
                buffer.push_back(u3 | 0x80);
              }
            }
          |
          ((hex_quad & (/D[89A-D]/i xdigit{2})) "\\u" (hex_quad & /D[C-F]/i xdigit{2}))
            %{
              u = ((u >> 16) - 0xD800) << 10 | ((u & 0xFFFF) - 0xDC00) | 0x010000;
              uint8_t u4 = u & 0x3F; u >>= 6;
              uint8_t u3 = u & 0x3F; u >>= 6;
              uint8_t u2 = u & 0x3F; u >>= 6;
              buffer.push_back(u  | 0xF0);
              buffer.push_back(u2 | 0x80);
              buffer.push_back(u3 | 0x80);
              buffer.push_back(u4 | 0x80);
            }
        );

      escape_sequence =
        ( "\\\"" @{ buffer.push_back('"'); }
        | "\\\\" @{ buffer.push_back('\\'); }
        | "\\/"  @{ buffer.push_back('/'); }
        | "\\b"  @{ buffer.push_back('\b'); }
        | "\\f"  @{ buffer.push_back('\f'); }
        | "\\n"  @{ buffer.push_back('\n'); }
        | "\\r"  @{ buffer.push_back('\r'); }
        | "\\t"  @{ buffer.push_back('\t'); }
        | unicode_escape_sequence
        );

      string_impl :=
          ( "\"" @{ lua_pushlstring(L, ps, 0); fret; }
          | unescaped+
            ( "\"" @{ lua_pushlstring(L, ps, p - ps); fret; }
            | escape_sequence >{ size_t size = p - ps; buffer.resize(size); memcpy(buffer.data(), ps, size); }
              ( escape_sequence
              | unescaped ${ buffer.push_back(fc); }
              )*
              "\"" @{ lua_pushlstring(L, buffer.data(), buffer.size()); fret; }
            )
          | escape_sequence >{ buffer.clear(); }
            ( escape_sequence
            | unescaped ${ buffer.push_back(fc); }
            )*
            "\"" @{ lua_pushlstring(L, buffer.data(), buffer.size()); fret; }
          );

      string = "\"" @{ ps = p; fcall string_impl; };

      value
        = "false" @{ lua_pushboolean(L, false); }
        | "null" @{ lua_pushnil(L); }
        | "true" @{ lua_pushboolean(L, true); }
        | "{" @{ lua_newtable(L); fcall object; }
        | "[" @{ lua_newtable(L); n = 0; fcall array; }
        | number
        | string;

      member = ws string ws ":" ws value %{ lua_settable(L, -3); };
      object := (member (ws "," member)*)? ws "}" @{ fret; };
      element = ws value >{ lua_pushinteger(L, ++n); } %{ lua_settable(L, -3); };
      array := (element (ws "," element)*)? ws "]" @{ fret; };
      main := ws value ws;

      write data noerror nofinal noentry;
    }%%


    void impl_decode(lua_State* L) {
      data_t data = check_data(L, 1);

      int cs = 0;
      int top = 0;
      %%write init;

      const char* p = data.data();
      const char* pe = p + data.size();
      const char* eof = pe;
      std::vector<int> stack; stack.reserve(16);

      const char* ps = nullptr;
      std::vector<char> buffer;
      lua_Integer n = 0;   // array index
      lua_Integer v = 0;   // integer value
      bool is_neg = false; // number is negative
      bool is_int = false; // number is integer
      uint32_t u = 0;      // unicode character sequence

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
