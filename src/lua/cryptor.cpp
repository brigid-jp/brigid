// Copyright (c) 2019-2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"
#include "scope_exit.hpp"
#include "stack_guard.hpp"
#include "thread_reference.hpp"
#include "view.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace brigid {
  namespace {
    crypto_cipher check_cipher(lua_State* L, int arg) {
      size_t size = 0;
      if (const char* data = lua_tolstring(L, arg, &size)) {
        std::string cipher(data, size);
        if (cipher == "aes-128-cbc") {
          return crypto_cipher::aes_128_cbc;
        } else if (cipher == "aes-192-cbc") {
          return crypto_cipher::aes_192_cbc;
        } else if (cipher == "aes-256-cbc") {
          return crypto_cipher::aes_256_cbc;
        }
      }
      luaL_argerror(L, arg, "unsupported cipher");
      throw BRIGID_LOGIC_ERROR("unreachable");
    }

    class cryptor_t : private noncopyable {
    public:
      cryptor_t(std::unique_ptr<cryptor>&& cryptor, thread_reference&& ref)
        : cryptor_(std::move(cryptor)),
          in_size_(),
          out_size_(),
          ref_(std::move(ref)),
          running_() {}

      void update(const char* in_data, size_t in_size, bool padding) {
        in_size_ += in_size;
        ensure_buffer_size(cryptor_->calculate_buffer_size(in_size_) - out_size_);
        size_t result = cryptor_->update(in_data, in_size, buffer_.data(), buffer_.size(), padding);
        out_size_ += result;
        if (result > 0) {
          if (lua_State* L = ref_.get()) {
            stack_guard guard(L);
            lua_pushvalue(L, 1);
            view_t* view = new_view(L, buffer_.data(), result);
            running_ = true;
            scope_exit scope_guard([&]() {
              running_ = false;
              view->close();
            });
            if (lua_pcall(L, 1, 0, 0) != 0) {
              throw BRIGID_RUNTIME_ERROR(lua_tostring(L, -1));
            }
          }
        }
      }

      void close() {
        cryptor_ = nullptr;
        in_size_ = 0;
        out_size_ = 0;
        ref_ = thread_reference();
      }

      bool closed() const {
        return !cryptor_;
      }

      bool running() const {
        return running_;
      }

    private:
      std::unique_ptr<cryptor> cryptor_;
      size_t in_size_;
      size_t out_size_;
      std::vector<char> buffer_;
      thread_reference ref_;
      bool running_;

      void ensure_buffer_size(size_t size) {
        if (buffer_.size() < size) {
          buffer_.resize(size);
        }
      }
    };

    cryptor_t* check_cryptor(lua_State* L, int arg, int validate = check_validate_all) {
      cryptor_t* self = check_udata<cryptor_t>(L, arg, "brigid.cryptor");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.cryptor");
        }
      }
      if (validate & check_validate_not_running) {
        if (self->running()) {
          luaL_error(L, "attempt to use a running brigid.cryptor");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_cryptor(L, 1, check_validate_none)->~cryptor_t();
    }

    void impl_close(lua_State* L) {
      cryptor_t* self = check_cryptor(L, 1, check_validate_not_running);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_update(lua_State* L) {
      cryptor_t* self = check_cryptor(L, 1);
      data_t source = check_data(L, 2);
      bool padding = lua_toboolean(L, 3) ? true : false;
      self->update(source.data(), source.size(), padding);
    }

    void impl_encryptor(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      data_t key = check_data(L, 2);
      data_t iv = check_data(L, 3);

      thread_reference ref;
      if (!lua_isnoneornil(L, 4)) {
        ref = thread_reference(L);
        lua_pushvalue(L, 4);
        lua_xmove(L, ref.get(), 1);
      }

      new_userdata<cryptor_t>(L, "brigid.cryptor",
          make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size()),
          std::move(ref));
    }

    void impl_decryptor(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      data_t key = check_data(L, 2);
      data_t iv = check_data(L, 3);

      thread_reference ref;
      if (!lua_isnoneornil(L, 4)) {
        ref = thread_reference(L);
        lua_pushvalue(L, 4);
        lua_xmove(L, ref.get(), 1);
      }

      new_userdata<cryptor_t>(L, "brigid.cryptor",
          make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size()),
          std::move(ref));
    }
  }

  void initialize_cryptor(lua_State* L) {
    try {
      open_cryptor();
    } catch (const std::exception& e) {
      luaL_error(L, "%s", e.what());
      return;
    }

    lua_newtable(L);
    {
      new_metatable(L, "brigid.cryptor");
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "__index");
      decltype(function<impl_gc>())::set_field(L, -1, "__gc");
      decltype(function<impl_close>())::set_field(L, -1, "__close");
      lua_pop(L, 1);

      decltype(function<impl_update>())::set_field(L, -1, "update");
      decltype(function<impl_close>())::set_field(L, -1, "close");
    }
    lua_setfield(L, -2, "cryptor");

    decltype(function<impl_encryptor>())::set_field(L, -1, "encryptor");
    decltype(function<impl_decryptor>())::set_field(L, -1, "decryptor");
  }
}
