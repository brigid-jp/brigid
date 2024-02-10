// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "data.hpp"
#include "error.hpp"
#include "function.hpp"

#include <lua.hpp>

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <sstream>
#include <type_traits>
#include <vector>

namespace brigid {
  namespace {
    using lua_unsigned_t = std::make_unsigned<lua_Integer>::type;
    static const size_t integer_digs = std::numeric_limits<lua_Integer>::digits10 + 1;
    static const lua_unsigned_t integer_max_div10 = std::numeric_limits<lua_Integer>::max() / 10;
    static const lua_unsigned_t integer_max_mod10 = std::numeric_limits<lua_Integer>::max() % 10;

    %%{
      machine json_parser;

      prepush { stack.push_back(0); }
      postpop { stack.pop_back(); }

      ws = [ \t\n\r]*;

      number =
        ( "-"?  ("0" | [1-9] digit*)
          ( "." @{ is_int = false; } digit+ ([eE]  [+\-]? digit+)?
          | ([eE] @{ is_int = false; } [+\-]? digit+)?
          )
        ) >{ ps = fpc; is_int = true; }
          %{
            lua_unsigned_t v = 0;
            lua_unsigned_t negative = 0;

            if (is_int) {
              const char* ptr = ps;
              if (*ptr == '-') {
                negative = 1;
                ++ptr;
              }
              size_t n = fpc - ptr;
              if (n < integer_digs) {
                for (; ptr != fpc; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
              } else if (n == integer_digs) {
                for (; ptr != fpc - 1; ++ptr) {
                  v *= 10;
                  v += *ptr - '0';
                }
                lua_unsigned_t u = *ptr - '0';
                if (v > integer_max_div10 || (v == integer_max_div10 && u > integer_max_mod10 + negative)) {
                  is_int = false;
                } else {
                  v *= 10;
                  v += u;
                }
              } else {
                is_int = false;
              }
            }

            if (is_int) {
              if (negative) {
                lua_pushinteger(L, 0 - v);
              } else {
                lua_pushinteger(L, v);
              }
            } else {
              // At the end-of-file, strtod() may not be able to find an
              // unrecognized character, because the null termination is not
              // guaranteed.
              // Also, The decimal point is denpended to the locale. For
              // example, the decimal point is ',' in the de_DE locale. In such
              // a case, strtod() may read too small or too much.
              do {
                if (fpc != eof && !decimal_point) {
                  char* end = nullptr;
                  double v = strtod(ps, &end);
                  if (end == fpc) {
                    lua_pushnumber(L, v);
                    break;
                  }
                }

                size_t n = fpc - ps;
                buffer.resize(n + 1);
                char* ptr = buffer.data();
                memcpy(ptr, ps, n);
                ptr[n] = '\0';

                if (!decimal_point) {
                  decimal_point = *localeconv()->decimal_point;
                }
                if (decimal_point != '.') {
                  if (char* q = strchr(ptr, '.')) {
                    *q = decimal_point;
                  }
                }

                char* end = nullptr;
                double v = strtod(ptr, &end);
                if (end == ptr + n) {
                  lua_pushnumber(L, v);
                  break;
                }

                std::ostringstream out;
                out << "cannot strtod at position " << (ps - pb + 1);
                throw BRIGID_RUNTIME_ERROR(out.str());
              } while (false);
            }
          };

      # Accept not valid UTF-8 characters
      unescaped_loose1 = [^\"\\] - 0x00..0x1F;
      unescaped_loose2 = [^\"\\];
      unescaped = unescaped_loose2;

      hex_quad =
        ( [0-9] @{ u <<= 4; u |= fc - '0'; }
        | [A-F] @{ u <<= 4; u |= fc - 'A' + 10; }
        | [a-f] @{ u <<= 4; u |= fc - 'a' + 10; }
        ){4};

      unicode_escape_sequence =
        "u" @{ u = 0; }
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
        ( "\"" @{ buffer.push_back('"'); }
        | "\\" @{ buffer.push_back('\\'); }
        | "/"  @{ buffer.push_back('/'); }
        | "b"  @{ buffer.push_back('\b'); }
        | "f"  @{ buffer.push_back('\f'); }
        | "n"  @{ buffer.push_back('\n'); }
        | "r"  @{ buffer.push_back('\r'); }
        | "t"  @{ buffer.push_back('\t'); }
        | unicode_escape_sequence
        );

      string_impl :=
        escape_sequence %{ ps = fpc; }
        ( "\"" @{ lua_pushlstring(L, buffer.data(), buffer.size()); fret; }
        | unescaped+
          ( "\"" @{ size_t m = buffer.size(); size_t n = fpc - ps; buffer.resize(m + n); char* ptr = buffer.data(); memcpy(ptr + m, ps, n); lua_pushlstring(L, ptr, m + n); fret; }
          | "\\" @{ size_t m = buffer.size(); size_t n = fpc - ps; buffer.resize(m + n); memcpy(buffer.data() + m, ps, n); fgoto string_impl; }
          )
        | "\\" @{ fgoto string_impl; }
        );

      string =
        "\"" @{ ps = fpc + 1; }
        ( "\"" @{ lua_pushlstring(L, ps, 0); }
        | unescaped+
          ( "\"" @{ lua_pushlstring(L, ps, fpc - ps); }
          | "\\" @{ size_t n = fpc - ps; buffer.resize(n); memcpy(buffer.data(), ps, n); fcall string_impl; }
          )
        | "\\" @{ buffer.clear(); fcall string_impl; }
        );

      value =
        ( "false" @{ lua_pushboolean(L, false); }
        | "null" @{ if (null_index) { lua_pushvalue(L, null_index); } else { lua_pushnil(L); } }
        | "true" @{ lua_pushboolean(L, true); }
        | "{" @{ lua_checkstack(L, 3); lua_createtable(L, 0, 8); fcall object; }
        | "[" @{ lua_checkstack(L, 2); lua_createtable(L, 8, 0); array_stack.push_back(0); fcall array; }
        | number
        | string
        );

      member = ws string ws ":" ws value %{ lua_rawset(L, -3); };
      object := (member (ws "," member)*)? ws "}" @{ fret; };
      element = ws value %{ lua_rawseti(L, -2, ++array_stack.back()); };
      array := (element (ws "," element)*)? ws "]" @{ lua_pushvalue(L, array_index); lua_setmetatable(L, -2); array_stack.pop_back(); fret; };
      main := ws value ws;

      write data noerror nofinal noentry;
    }%%

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    int impl_parse(lua_State* L) {
      data_t data = check_data(L, 1);

      int cs = 0;
      int top = lua_gettop(L);

      int null_index = top >= 2 ? 2 : 0;
      luaL_getmetatable(L, "brigid.json.array");
      int array_index = top + 1;

      %%write init;

      const char* const pb = data.data();
      const char* p = pb;
      const char* const pe = p + data.size();
      const char* const eof = pe;
      std::vector<int> stack; stack.reserve(16);

      const char* ps = nullptr;
      std::vector<char> buffer;
      std::vector<int> array_stack;
      bool is_int = false;    // number is integer
      char decimal_point = 0; // *localeconv()->decimal_point
      uint32_t u = 0;         // unicode escape sequence

      %%write exec;

      if (cs >= %%{ write first_final; }%% && stack.empty()) {
        return 1;
      }

      std::ostringstream out;
      out << "cannot parse json at position " << (p - pb + 1);
      throw BRIGID_RUNTIME_ERROR(out.str());
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
  }

  void initialize_json_parse(lua_State* L) {
    decltype(function<impl_parse>())::set_field(L, -1, "parse");
  }
}
