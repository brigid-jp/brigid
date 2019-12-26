// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"
#include "scope_exit.hpp"
#include "view.hpp"

#include <stddef.h>
#include <memory>
#include <utility>
#include <string>
#include <vector>

namespace brigid {
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
      throw BRIGID_LOGIC_ERROR("unsupported cipher");
    }

    crypto_hash check_hash(lua_State* L, int arg) {
      std::string hash = check_data(L, arg).str();
      if (hash == "sha256") {
        return crypto_hash::sha256;
      } else if (hash == "sha512") {
        return crypto_hash::sha512;
      }
      throw BRIGID_LOGIC_ERROR("unsupported cipher");
    }

    class cryptor_t : private noncopyable {
    public:
      cryptor_t(std::unique_ptr<cryptor>&& cryptor, reference&& write_cb)
        : cryptor_(std::move(cryptor)),
          in_size_(),
          out_size_(),
          write_cb_(std::move(write_cb)),
          running_() {}

      void update(const char* in_data, size_t in_size, bool padding) {
        in_size_ += in_size;
        ensure_buffer_size(cryptor_->calculate_buffer_size(in_size_) - out_size_);
        size_t result = cryptor_->update(in_data, in_size, buffer_.data(), buffer_.size(), padding);
        out_size_ += result;
        if (result > 0) {
          if (lua_State* L = write_cb_.state()) {
            stack_guard guard(L);
            write_cb_.get_field(L);
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
        write_cb_ = reference();
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
      reference write_cb_;
      bool running_;

      void ensure_buffer_size(size_t size) {
        if (buffer_.size() < size) {
          buffer_.resize(size);
        }
      }
    };

    class hasher_t : private noncopyable {
    public:
      explicit hasher_t(std::unique_ptr<hasher>&& hasher)
        : hasher_(std::move(hasher)) {}

      void update(const char* data, size_t size) {
        hasher_->update(data, size);
      }

      std::vector<char> digest() {
        return hasher_->digest();
      }

      void close() {
        hasher_ = nullptr;
      }

      bool closed() const {
        return !hasher_;
      }

    private:
      std::unique_ptr<hasher> hasher_;
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

    hasher_t* check_hasher(lua_State* L, int arg, int validate = check_validate_all) {
      hasher_t* self = check_udata<hasher_t>(L, arg, "brigid.hasher");
      if (validate & check_validate_not_closed) {
        if (self->closed()) {
          luaL_error(L, "attempt to use a closed brigid.hasher");
        }
      }
      return self;
    }

    void impl_cryptor_gc(lua_State* L) {
      check_cryptor(L, 1, check_validate_none)->~cryptor_t();
    }

    void impl_cryptor_close(lua_State* L) {
      cryptor_t* self = check_cryptor(L, 1, check_validate_not_running);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_cryptor_update(lua_State* L) {
      cryptor_t* self = check_cryptor(L, 1);
      data_t source = check_data(L, 2);
      bool padding = lua_toboolean(L, 3);
      self->update(source.data(), source.size(), padding);
    }

    void impl_hasher_gc(lua_State* L) {
      check_hasher(L, 1, check_validate_none)->~hasher_t();
    }

    void impl_hasher_close(lua_State* L) {
      hasher_t* self = check_hasher(L, 1, check_validate_none);
      if (!self->closed()) {
        self->close();
      }
    }

    void impl_hasher_call(lua_State* L) {
      crypto_hash hash = check_hash(L, 2);
      new_userdata<hasher_t>(L, "brigid.hasher", make_hasher(hash));
    }

    void impl_hasher_update(lua_State* L) {
      hasher_t* self = check_hasher(L, 1);
      data_t source = check_data(L, 2);
      self->update(source.data(), source.size());
    }

    void impl_hasher_digest(lua_State* L) {
      hasher_t* self = check_hasher(L, 1);
      std::vector<char> result = self->digest();
      push(L, result.data(), result.size());
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
      set_field(L, -1, "__gc", impl_cryptor_gc);
      set_field(L, -1, "__close", impl_cryptor_close);
      lua_pop(L, 1);

      set_field(L, -1, "update", impl_cryptor_update);
      set_field(L, -1, "close", impl_cryptor_close);
    }
    set_field(L, -2, "cryptor");

    lua_newtable(L);
    {
      luaL_newmetatable(L, "brigid.hasher");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_hasher_gc);
      set_field(L, -1, "__close", impl_hasher_close);
      lua_pop(L, 1);

      set_metafield(L, -1, "__call", impl_hasher_call);
      set_field(L, -1, "update", impl_hasher_update);
      set_field(L, -1, "digest", impl_hasher_digest);
      set_field(L, -1, "close", impl_hasher_close);
    }
    set_field(L, -2, "hasher");

    set_field(L, -1, "encryptor", impl_encryptor);
    set_field(L, -1, "decryptor", impl_decryptor);
  }
}
