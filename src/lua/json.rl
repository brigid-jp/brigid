// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <stdlib.h>
#include <string.h>

#include <sstream>
#include <limits>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace {
    using lua_unsigned_t = std::make_unsigned<lua_Integer>::type;
    static const lua_unsigned_t integer_max_div10 = std::numeric_limits<lua_Integer>::max() / 10;
    static const lua_unsigned_t integer_max_mod10 = std::numeric_limits<lua_Integer>::max() % 10;

    %%{
      machine json_parser;

      prepush { stack.push_back(0); }
      postpop { stack.pop_back(); }

      ws = [ \t\n\r]*;

      number =
        ( "-"? @{ is_neg = 1; }
          ( "0" @{ v = 0; }
          | [1-9] @{ v = fc - '0'; }
            digit*
              ${
                lua_unsigned_t u = fc - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + is_neg)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              }
          )
          ("." digit+)? @{ is_int = false; }
          ([eE] [+\-]? digit+)? @{ is_int = false; }
        ) >{
            ps = fpc;
            is_neg = 0;
            is_int = true;
          }
          %{
            if (is_int) {
              if (is_neg) {
                lua_pushinteger(L, -v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              size_t size = fpc - ps;
              buffer.resize(size + 1);
              char* ptr = buffer.data();
              char* end = nullptr;
              memcpy(ptr, ps, size);
              ptr[size] = '\0';
              double u = strtod(ptr, &end);
              if (end != ptr + size) {
                // You may try to translate '.' to ',' if locale is de_DE
                throw BRIGID_RUNTIME_ERROR("cannot strtod");
              }
              lua_pushnumber(L, u);
            }
          };

      unescaped = 0x20 | 0x21 | 0x23..0x5B | 0x5D..0x7F | 0x80..0xFF;

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
          ( "\"" @{ lua_pushlstring(L, ps, fpc - ps); fret; }
          | escape_sequence >{ size_t size = fpc - ps; buffer.resize(size); memcpy(buffer.data(), ps, size); }
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

      string = "\"" @{ ps = fpc + 1; fcall string_impl; };

      value =
        ( "false" @{ lua_pushboolean(L, false); }
        | "null" @{ if (null_index < 0) { lua_pushnil(L); } else { lua_pushvalue(L, null_index); } }
        | "true" @{ lua_pushboolean(L, true); }
        | "{" @{ lua_newtable(L); fcall object; }
        | "[" @{ lua_newtable(L); n.push_back(0); fcall array; }
        | number
        | string
        );

      member = ws string ws ":" ws value %{ lua_settable(L, -3); };
      object := (member (ws "," member)*)? ws "}" @{ fret; };
      element = ws value >{ lua_pushinteger(L, ++n.back()); } %{ lua_settable(L, -3); };
      array := (element (ws "," element)*)? ws "]" @{ n.pop_back(); fret; };
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
      int null_index = lua_gettop(L) >= 2 ? 2 : -1;
      std::vector<lua_Integer> n; // array index
      lua_unsigned_t v = 0;       // integer
      lua_unsigned_t is_neg = 0;  // number is negative
      bool is_int = false;        // number is integer
      uint32_t u = 0;             // unicode escape sequence

      %%write exec;

      if (cs < %%{ write first_final; }%% || !stack.empty()) {
        std::ostringstream out;
        out << "parser error at position " << (p - data.data());
        throw BRIGID_RUNTIME_ERROR(out.str());
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
