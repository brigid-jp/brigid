// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/error.hpp>
#include <brigid/noncopyable.hpp>
#include "common.hpp"
#include "data.hpp"
#include "util_lua.hpp"
#include "view.hpp"

#include <utility>
#include <vector>

namespace brigid {
  using namespace lua;

  namespace {
    crypto_cipher check_cipher(lua_State* L, int arg) {
      const auto cipher = luax_check_data(L, arg).to_str();
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
      cryptor_t(std::unique_ptr<cryptor>&& cryptor, size_t max_size, reference&& write_cb)
        : cryptor_(std::move(cryptor)),
          out_size_(),
          max_size_(max_size),
          write_cb_(std::move(write_cb)) {}

      void update(const char* in_data, size_t in_size, bool padding) {
        max_size_ += in_size;
        ensure_buffer_size(max_size_ - out_size_);
        size_t result = cryptor_->update(in_data, in_size, buffer_.data(), buffer_.size(), padding);
        out_size_ += result;

        lua_State* L = write_cb_.state();
        {
          top_saver saver(L);
          write_cb_.get_field(L);
          view_invalidator invalidator(new_view(L, buffer_.data(), result));
          if (lua_pcall(L, 1, 0, 0) != 0) {
            throw BRIGID_ERROR(lua_tostring(L, -1));
          }
        }
      }

    private:
      std::unique_ptr<cryptor> cryptor_;
      size_t out_size_;
      size_t max_size_;
      std::vector<char> buffer_;
      reference write_cb_;

      void ensure_buffer_size(size_t size) {
        if (buffer_.size() < size) {
          buffer_.resize(size);
        }
      }
    };

    cryptor_t* check_cryptor(lua_State* L, int arg) {
      return check_udata<cryptor_t>(L, arg, "brigid.cryptor");
    }

    void impl_gc(lua_State* L) {
      check_cryptor(L, -1)->~cryptor_t();
    }

    void impl_update(lua_State* L) {
      cryptor_t* self = check_cryptor(L, 1);
      const auto in = check_data(L, 2);
      bool padding = lua_toboolean(L, 3);
      self->update(in.data(), in.size(), padding);
    }

    void impl_encryptor(lua_State* L) {
      const auto cipher = check_cipher(L, 1);
      const auto key = check_data(L, 2);
      const auto iv = check_data(L, 3);
      luaL_checkany(L, 4);
      new_userdata<cryptor_t>(L, "brigid.cryptor", make_encryptor(cipher, key.data(), key.size(), iv.data(), iv.size()), get_block_size(cipher), reference(L, 4));
    }

    void impl_decryptor(lua_State* L) {
      const auto cipher = check_cipher(L, 1);
      const auto key = check_data(L, 2);
      const auto iv = check_data(L, 3);
      luaL_checkany(L, 4);
      new_userdata<cryptor_t>(L, "brigid.cryptor", make_decryptor(cipher, key.data(), key.size(), iv.data(), iv.size()), 0, reference(L, 4));
    }
  }

  void initialize_crypto(lua_State* L) {
    lua_newtable(L);
    {
      top_saver saver(L);
      luaL_newmetatable(L, "brigid.cryptor");
      lua_pushvalue(L, -2);
      set_field(L, -2, "__index");
      set_field(L, -1, "__gc", impl_gc);
    }
    {
      set_field(L, -1, "update", impl_update);
    }
    set_field(L, -1, "cryptor");

    set_field(L, -1, "encryptor", impl_encryptor);
    set_field(L, -1, "decryptor", impl_decryptor);
  }
}
