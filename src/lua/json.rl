// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"

#include <lua.hpp>

#include <utility>
#include <vector>

namespace brigid {
  namespace {
    class thread : private noncopyable {
    public:
      thread()
        : state_(),
          state_ref_(LUA_NOREF) {}

      explicit thread(lua_State* L)
        : state_(),
          state_ref_(LUA_NOREF) {
        state_ = lua_newthread(L);
        state_ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
      }

      thread(thread&& that)
        : state_(that.state_),
          state_ref_(that.state_ref_) {
        that.reset();
      }

      ~thread() {
        unref();
      }

      thread& operator=(thread&& that) {
        if (this != &that) {
          unref();
          state_ = that.state_;
          state_ref_ = that.state_ref_;
          that.reset();
        }
        return *this;
      }

      void close() {
        if (lua_State* L = state_) {
          luaL_unref(L, LUA_REGISTRYINDEX, state_ref_);
          state_ = nullptr;
          state_ref_ = LUA_NOREF;
        }
      }

      lua_State* state() const {
        return state_;
      }

    private:
      lua_State* state_;
      int state_ref_;

      void unref() {
        if (lua_State* L = state_) {
          luaL_unref(L, LUA_REGISTRYINDEX, state_ref_);
          reset();
        }
      }

      void reset() {
        state_ = nullptr;
        state_ref_ = LUA_NOREF;
      }
    };

    enum class json_parser_state { start, running, accept, error };

    %%{
      machine json_parser;

      prepush { stack.resize(stack.size() + 1); }
      postpop { stack.resize(stack.size() - 1); }

      ws = [ \t\n\r]*;

      number
        = "-"? (0 | [1-9] digit*) ("." digit+)? ([eE] [+\-]? digit+)?;

      utf8_tail = 0x80..0xBF;

      string
        = "\""
          (
            ( "\\\""
            | "\\\\"
            | "\\/"
            | "\\b"
            | "\\f"
            | "\\n"
            | "\\r"
            | "\\t"
            | "\\u" xdigit{4}
            )
          [^\\"]
#          | 0x00..0x7F
#          | 0xC2..0xDF utf8_tail
#          | 0xE0 0xA0..0xBF utf8_tail
#          | 0xE1..0xEC utf8_tail{2}
#          | 0xED 0x80..0x9F utf8_tail
#          | 0xEE..0xEF utf8_tail{2}
#          | 0xF0 0x90..0xBF utf8_tail{2}
#          | 0xF1..0xF3 utf8_tail{3}
#          | 0xF4 0x80..0x8F utf8_tail{2}
          )*
          "\"";

      value
        = "false" @{ lua_pushboolean(L, false); }
        | "null" @{ lua_pushnil(L); }
        | "true" @{ lua_pushboolean(L, true); }
        | "{" @{ lua_newtable(L); fcall object; }
        | "["
            @{
              lua_newtable(L);
              n_ = 0;
              fcall array;
            }
        | number
        | string;

      member = ws string ws ":" ws value;

      object := (member (ws "," ws member)*)? ws "}" @{ fret; };

      array :=
        (
          value
            @{ lua_seti(L, -2, ++n_); }
          (
            ws "," ws
            value
          )*
        )? ws
        "]" @{ fret; };

      main := ws value ws;

      write data noerror nofinal noentry;
    }%%

    class json_parser_t : private noncopyable {
    public:
      explicit json_parser_t(thread&& thread)
        : thread_(std::move(thread)),
          state_(json_parser_state::start) {
        %%write init;
      }

      const char* parse(const char* data, size_t size) {
        if (lua_State* L = thread_.state()) {
          const char* p = data;
          const char* pe = data + size;

          %%write exec;
        }


        // switch (state_) {
        //   case json_parser_state::start;
        //   case json_parser_state::running:
        //     break;
        //   case json_parser_state::accept:
        //     break;
        //   case json_parser_state::error:
        //     break;
        // }
        return nullptr;
      }

      void close() {
        thread_ = thread();
      }

      bool closed() const {
        return !thread_.state();
      }

      json_parser_state state() const {
        return state_;
      }

    private:
      thread thread_;
      json_parser_state state_;

      int cs;
      std::vector<int> stack;
      int top;

      lua_Integer n_;
    };

    json_parser_t* check_json_parser(lua_State* L, int arg, int validate = check_validate_all) {
      json_parser_t* self = check_udata<json_parser_t>(L, arg, "brigid.json_parser");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.json_parser");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_json_parser(L, 1, check_validate_none)->~json_parser_t();
    }

    void impl_close(lua_State* L) {
      json_parser_t* self = check_json_parser(L, 1, check_validate_none);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_call(lua_State* L) {
      new_userdata<json_parser_t>(L, "brigid.json_writer", thread(L));
    }

    void impl_parse(lua_State* L) {
      json_parser_t* self = check_json_parser(L, 1);
      data_t data = check_data(L, 2);
      self->parse(data.data(), data.size());
    }
  }

  void initialize_json(lua_State* L) {
    lua_newtable(L);
    {
      luaL_newmetatable(L, "brigid.json_parser");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      set_field(L, -1, "__close", impl_close);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_call);
      set_field(L, -1, "parse", impl_parse);
    }
    set_field(L, -2, "json_parser");
  }
}
