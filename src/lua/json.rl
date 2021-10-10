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

      action utf8_1 {
        buffer.push_back(u);
      }

      action utf8_2 {
        uint8_t u2 = u & 0x3F; u >>= 6;
        buffer.push_back(u  | 0xC0);
        buffer.push_back(u2 | 0x80);
      }

      action utf8_3 {
        uint8_t u3 = u & 0x3F; u >>= 6;
        uint8_t u2 = u & 0x3F; u >>= 6;
        buffer.push_back(u  | 0xE0);
        buffer.push_back(u2 | 0x80);
        buffer.push_back(u3 | 0x80);
      }

      action utf8_4 {
        u = ((u >> 16) - 0xD800) << 10 | ((u & 0xFFFF) - 0xDC00) | 0x010000;
        uint8_t u4 = u & 0x3F; u >>= 6;
        uint8_t u3 = u & 0x3F; u >>= 6;
        uint8_t u2 = u & 0x3F; u >>= 6;
        buffer.push_back(u  | 0xF0);
        buffer.push_back(u2 | 0x80);
        buffer.push_back(u3 | 0x80);
        buffer.push_back(u4 | 0x80);
      }

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
                // TODO 整数の範囲内か確かめる: Lua 5.3以降のコードを確認
                // TODO LuaJITは？
                lua_pushinteger(L, is_minus ? -v : v);
              } else {
                // 入力文字列が\0もしくは他の区切り文字で終端していることは保証されない
                size = p - ps;
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

      unescaped =
        # UTF8-1
        ( 0x20 | 0x21 | 0x23..0x5B | 0x5D..0x7F
        # UTF8-2
        | 0xC2..0xDF 0x80..0xBF
        # UTF8-3
        | 0xE0 0xA0..0xBF 0x80..0xBF | 0xE1..0xEC 0x80..0xBF{2}
        | 0xED 0x80..0x9F 0x80..0xBF | 0xEE..0xEF 0x80..0xBF{2}
        # UTF8-4
        | 0xF0 0x90..0xBF 0x80..0xBF{2} | 0xF1..0xF3 0x80..0xBF{3}
        | 0xF4 0x80..0x8F 0x80..0xBF{2}
        );

      hex_quad =
        ( [0-9] @{ u <<= 4; u |= fc - '0'; }
        | [A-F] @{ u <<= 4; u |= fc - 'A' + 10; }
        | [a-f] @{ u <<= 4; u |= fc - 'a' + 10; }
        ){4};

      # U+0000..U+007F
      hex1 = /00[0-7][0-9A-F]/i;

      # U+0080..U+07FF
      hex2 =
        ( /00[89A-F][0-9A-F]/i
        | /0[1-7][0-9A-F][0-9A-F]/i
        );

      # U+0800..U+D7FF, U+E000..U+FFFF
      hex3 =
        ( /0[89A-F][0-9A-F][0-9A-F]/i
        | /[1-9A-C][0-9A-F][0-9A-F][0-9A-F]/i
        | /D[0-7][0-9A-F][0-9A-F]/i
        | /[EF][0-9A-F][0-9A-F][0-9A-F]/i
        );

      # U+D800..U+DBFF
      hex4h = /D[89AB][0-9A-F][0-9A-F]/i;

      # U+DC00..U+DFFF
      hex4l = /D[C-F][0-9A-F][0-9A-F]/i;

      unicode_escape_sequence =
        "\\u" @{ u = 0; }
        ( (hex_quad & hex1) %utf8_1
        | (hex_quad & hex2) %utf8_2
        | (hex_quad & hex3) %utf8_3
        | (hex_quad & hex4h) "\\u" (hex_quad & hex4l) %utf8_4
        );

      escape_sequence =
        ( "\\\"" @{ buffer.push_back('"'); }
        | "\\\\" @{ buffer.push_back('\\'); }
        | "\\/" @{ buffer.push_back('/'); }
        | "\\b" @{ buffer.push_back('\b'); }
        | "\\f" @{ buffer.push_back('\f'); }
        | "\\n" @{ buffer.push_back('\n'); }
        | "\\r" @{ buffer.push_back('\r'); }
        | "\\t" @{ buffer.push_back('\t'); }
        | unicode_escape_sequence
        );

      string
        = "\"" %{ ps = p; }
          ( "\"" @{ lua_pushlstring(L, ps, 0); }
          | unescaped+
            ( "\"" @{ lua_pushlstring(L, ps, p - ps); }
            | escape_sequence >{ size = p - ps; buffer.resize(size); memcpy(buffer.data(), ps, size); }
              ( escape_sequence
              | unescaped ${ buffer.push_back(fc); }
              )*
              "\"" @{ lua_pushlstring(L, buffer.data(), buffer.size()); }
            )
          | escape_sequence >{ buffer.clear(); }
            ( escape_sequence
            | unescaped ${ buffer.push_back(fc); }
            )*
            "\"" @{ lua_pushlstring(L, buffer.data(), buffer.size()); }
          );

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

      int cs;
      int top;
      %%write init;

      const char* p = data.data();
      const char* pe = p + data.size();
      const char* eof = pe;
      std::vector<int> stack;
      stack.reserve(16);

      const char* ps; // 先頭ポインタの保存
      lua_Integer n; // arrayのインデックス
      lua_Integer v; // 整数の保持
      std::vector<char> buffer; // reserveしない
      bool is_minus; // 数の符号
      bool is_integer; // 数の種別
      size_t size; // バッファサイズ計算用
      uint32_t u; // unicode

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
