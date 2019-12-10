// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "data.hpp"
#include "util_lua.hpp"
#include "view.hpp"

#include <stddef.h>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace brigid {
  using namespace lua;

  namespace {
    crypto_cipher check_cipher(lua_State* L, int arg) {
      std::string cipher = check_data(L, arg).str();
      if (cipher == "aes-128-cbc") {
        return crypto_cipher::aes_128_cbc;
      } else if (cipher == "aes-192-cbc") {
        return crypto_cipher::aes_192_cbc;
      } else if (cipher == "aes-256-cbc") {
        return crypto_cipher::aes_256_cbc;
      }
      throw BRIGID_ERROR("unsupported cipher");
    }

    class cryptor_t : private noncopyable {
    public:
      cryptor_t(std::unique_ptr<cryptor>&& cryptor, reference&& write_cb)
        : cryptor_(std::move(cryptor)),
          in_size_(),
          out_size_(),
          write_cb_(std::move(write_cb)) {}

      void update(const char* in_data, size_t in_size, bool padding) {
        in_size_ += in_size;
        ensure_buffer_size(cryptor_->calculate_buffer_size(in_size_) - out_size_);
        size_t result = cryptor_->update(in_data, in_size, buffer_.data(), buffer_.size(), padding);
        out_size_ += result;
        if (result > 0) {
          if (lua_State* L = write_cb_.state()) {
            stack_guard guard(L);
            write_cb_.get_field(L);
            view_guard vguard(new_view(L, buffer_.data(), result));
            if (lua_pcall(L, 1, 0, 0) != 0) {
              throw BRIGID_ERROR(lua_tostring(L, -1));
            }
          }
        }
      }

      void close() {
        cryptor_ = nullptr;
        in_size_ = 0;
        out_size_ = 0;
        write_cb_ = reference();
      }

      bool closed() const {
        return !cryptor_;
      }

    private:
      std::unique_ptr<cryptor> cryptor_;
      size_t in_size_;
      size_t out_size_;
      std::vector<char> buffer_;
      reference write_cb_;

      void ensure_buffer_size(size_t size) {
        if (buffer_.size() < size) {
          buffer_.resize(size);
        }
      }
    };

    cryptor_t* check_cryptor(lua_State* L, int arg, bool check_closed = true) {
      cryptor_t* self = check_udata<cryptor_t>(L, arg, "brigid.cryptor");
      if (check_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.cryptor");
        }
      }
      return self;
    }

    void impl_gc(lua_State* L) {
      check_cryptor(L, -1, false)->~cryptor_t();
    }

    void impl_update(lua_State* L) {
      cryptor_t* self = check_cryptor(L, 1);
      const auto in = check_data(L, 2);
      bool padding = lua_toboolean(L, 3);
      self->update(in.data(), in.size(), padding);
    }

    void impl_close(lua_State* L) {
      check_cryptor(L, 1)->close();
    }

    void impl_encryptor(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      data_t key = check_data(L, 2);
      data_t iv = check_data(L, 3);
      reference write_cb;

      if (!lua_isnoneornil(L, 4)) {
        write_cb = reference(L, 4);
      }

      new_userdata<cryptor_t>(L, "brigid.cryptor",
          make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size()),
          std::move(write_cb));
    }

    void impl_decryptor(lua_State* L) {
      crypto_cipher cipher = check_cipher(L, 1);
      data_t key = check_data(L, 2);
      data_t iv = check_data(L, 3);
      reference write_cb;

      if (!lua_isnoneornil(L, 4)) {
        write_cb = reference(L, 4);
      }

      new_userdata<cryptor_t>(L, "brigid.cryptor",
          make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size()),
          std::move(write_cb));
    }
  }

  void initialize_crypto(lua_State* L) {
    lua_newtable(L);
    {
      luaL_newmetatable(L, "brigid.cryptor");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
      lua_pop(L, 1);

      set_field(L, -1, "update", impl_update);
      set_field(L, -1, "close", impl_close);
    }
    set_field(L, -2, "cryptor");

    set_field(L, -1, "encryptor", impl_encryptor);
    set_field(L, -1, "decryptor", impl_decryptor);
  }
}
