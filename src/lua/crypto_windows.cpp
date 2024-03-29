// Copyright (c) 2021,2022,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "common_windows.hpp"
#include "crypto.hpp"
#include "error.hpp"
#include "noncopyable.hpp"
#include "type_traits.hpp"

#include <lua.hpp>

#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>

#include <stddef.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace brigid {
  namespace {
    void check(NTSTATUS code) {
      if (!BCRYPT_SUCCESS(code)) {
        std::string message;
        if (get_error_message("ntdll.dll", code, message)) {
          throw BRIGID_RUNTIME_ERROR(message, make_error_code("bcrypt error", code));
        } else {
          throw BRIGID_RUNTIME_ERROR(make_error_code("bcrypt error", code));
        }
      }
    }

    void close_algorithm_provider(BCRYPT_ALG_HANDLE handle) {
      BCryptCloseAlgorithmProvider(handle, 0);
    }

    using alg_handle_t = std::unique_ptr<remove_pointer_t<BCRYPT_ALG_HANDLE>, decltype(&close_algorithm_provider)>;

    alg_handle_t make_alg_handle(BCRYPT_ALG_HANDLE handle = nullptr) {
      return alg_handle_t(handle, &close_algorithm_provider);
    }

    using key_handle_t = std::unique_ptr<remove_pointer_t<BCRYPT_KEY_HANDLE>, decltype(&BCryptDestroyKey)>;

    key_handle_t make_key_handle(BCRYPT_KEY_HANDLE handle = nullptr) {
      return key_handle_t(handle, &BCryptDestroyKey);
    }

    using hash_handle_t = std::unique_ptr<remove_pointer_t<BCRYPT_HASH_HANDLE>, decltype(&BCryptDestroyHash)>;

    hash_handle_t make_hash_handle(BCRYPT_HASH_HANDLE handle = nullptr) {
      return hash_handle_t(handle, &BCryptDestroyHash);
    }

    class aes_cryptor_impl : public cryptor {
    public:
      aes_cryptor_impl(const char* key_data, size_t key_size, const char* iv_data, thread_reference&& ref)
        : cryptor(std::move(ref)),
          alg_(make_alg_handle()),
          key_(make_key_handle()),
          iv_(16),
          in_buffer_(16),
          in_position_() {
        BCRYPT_ALG_HANDLE alg = nullptr;
        check(BCryptOpenAlgorithmProvider(
            &alg,
            BCRYPT_AES_ALGORITHM,
            nullptr,
            0));
        alg_ = make_alg_handle(alg);

        check(BCryptSetProperty(
            alg_.get(),
            BCRYPT_CHAINING_MODE,
            reinterpret_cast<PUCHAR>(BCRYPT_CHAIN_MODE_CBC),
            sizeof(BCRYPT_CHAIN_MODE_CBC),
            0));

        DWORD size = 0;
        DWORD result = 0;
        check(BCryptGetProperty(
            alg_.get(),
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&size),
            sizeof(size),
            &result,
            0));
        key_buffer_.resize(size);

        BCRYPT_KEY_HANDLE key = nullptr;
        check(BCryptGenerateSymmetricKey(
            alg_.get(),
            &key,
            key_buffer_.data(),
            static_cast<ULONG>(key_buffer_.size()),
            reinterpret_cast<PUCHAR>(const_cast<char*>(key_data)),
            static_cast<ULONG>(key_size),
            0));
        key_ = make_key_handle(key);

        memmove(iv_.data(), iv_data, 16);
      }

      virtual size_t impl_update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        size_t result = 0;

        if (padding) {
          if (in_position_ > 0) {
            size_t size = std::min(in_size, in_buffer_.size() - in_position_);
            memmove(in_buffer_.data() + in_position_, in_data, size);
            in_data += size;
            in_size -= size;
            in_position_ += size;

            if (in_size == 0) {
              result = impl_execute(key_.get(), iv_, in_buffer_.data(), in_position_, out_data, out_size, true);
              in_position_ = 0;
            } else {
              result = impl_execute(key_.get(), iv_, in_buffer_.data(), in_position_, out_data, out_size, false);
              in_position_ = 0;
              result += impl_execute(key_.get(), iv_, in_data, in_size, out_data + result, out_size - result, true);
            }
          } else {
            result = impl_execute(key_.get(), iv_, in_data, in_size, out_data, out_size, true);
          }
        } else {
          if (in_position_ > 0) {
            size_t size = std::min(in_size, in_buffer_.size() - in_position_);
            memmove(in_buffer_.data() + in_position_, in_data, size);
            in_data += size;
            in_size -= size;
            in_position_ += size;

            if (in_position_ == in_buffer_.size()) {
              result = impl_execute(key_.get(), iv_, in_buffer_.data(), in_position_, out_data, out_size, false);
              in_position_ = 0;
            }
          }

          size_t size = in_size - in_size % 16;
          if (size > 0) {
            result += impl_execute(key_.get(), iv_, in_data, size, out_data + result, out_size - result, false);
            in_data += size;
            in_size -= size;
          }

          if (in_size > 0) {
            memmove(in_buffer_.data() + in_position_, in_data, in_size);
            in_position_ += in_size;
          }
        }

        return result;
      }

      virtual void impl_close() {
        alg_ = make_alg_handle();
        key_ = make_key_handle();
      }

      virtual size_t impl_execute(BCRYPT_KEY_HANDLE, std::vector<UCHAR>&, const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) = 0;

    private:
      alg_handle_t alg_;
      std::vector<UCHAR> key_buffer_;
      key_handle_t key_;
      std::vector<UCHAR> iv_;
      std::vector<char> in_buffer_;
      size_t in_position_;
    };

    class aes_encryptor_impl : public aes_cryptor_impl, private noncopyable {
    public:
      aes_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, thread_reference&& ref)
        : aes_cryptor_impl(key_data, key_size, iv_data, std::move(ref)) {}

      virtual size_t impl_calculate_buffer_size(size_t in_size) const {
        return in_size + 16;
      };

      virtual size_t impl_execute(BCRYPT_KEY_HANDLE key, std::vector<UCHAR>& iv, const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        ULONG result = 0;
        check(BCryptEncrypt(
            key,
            reinterpret_cast<PUCHAR>(const_cast<char*>(in_data)),
            static_cast<ULONG>(in_size),
            nullptr,
            iv.data(),
            static_cast<ULONG>(iv.size()),
            reinterpret_cast<PUCHAR>(out_data),
            static_cast<ULONG>(out_size),
            &result,
            padding ? BCRYPT_BLOCK_PADDING : 0));
        return result;
      }
    };

    class aes_decryptor_impl : public aes_cryptor_impl, private noncopyable {
    public:
      aes_decryptor_impl(const char* key_data, size_t key_size, const char* iv_data, thread_reference&& ref)
        : aes_cryptor_impl(key_data, key_size, iv_data, std::move(ref)) {}

      virtual size_t impl_calculate_buffer_size(size_t in_size) const {
        return in_size;
      };

      virtual size_t impl_execute(BCRYPT_KEY_HANDLE key, std::vector<UCHAR>& iv, const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        ULONG result = 0;
        check(BCryptDecrypt(
            key,
            reinterpret_cast<PUCHAR>(const_cast<char*>(in_data)),
            static_cast<ULONG>(in_size),
            nullptr,
            iv.data(),
            static_cast<ULONG>(iv.size()),
            reinterpret_cast<PUCHAR>(out_data),
            static_cast<ULONG>(out_size),
            &result,
            padding ? BCRYPT_BLOCK_PADDING : 0));
        return result;
      }
    };

    template <size_t T_size>
    class hasher_impl : public hasher, private noncopyable {
    public:
      explicit hasher_impl(LPCWSTR algorithm)
        : alg_(make_alg_handle()),
          hash_(make_hash_handle()) {
        BCRYPT_ALG_HANDLE alg = nullptr;
        check(BCryptOpenAlgorithmProvider(
            &alg,
            algorithm,
            nullptr,
            0));
        alg_ = make_alg_handle(alg);

        DWORD size = 0;
        DWORD result = 0;
        check(BCryptGetProperty(
            alg_.get(),
            BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&size),
            sizeof(size),
            &result,
            0));
        hash_buffer_.resize(size);

        BCRYPT_HASH_HANDLE hash = nullptr;
        check(BCryptCreateHash(
            alg_.get(),
            &hash,
            hash_buffer_.data(),
            static_cast<ULONG>(hash_buffer_.size()),
            nullptr,
            0,
            0));
        hash_ = make_hash_handle(hash);
      }

      virtual void update(const char* data, size_t size) {
        check(BCryptHashData(
            hash_.get(),
            reinterpret_cast<PUCHAR>(const_cast<char*>(data)),
            static_cast<ULONG>(size),
            0));
      }

      virtual void digest(lua_State* L) {
        DWORD size = 0;
        DWORD result = 0;
        check(BCryptGetProperty(
            alg_.get(),
            BCRYPT_HASH_LENGTH,
            reinterpret_cast<PUCHAR>(&size),
            sizeof(size),
            &result,
            0));
        if (size != T_size) {
          throw BRIGID_LOGIC_ERROR("invalid buffer size");
        }
        char buffer[T_size] = {};
        check(BCryptFinishHash(
            hash_.get(),
            reinterpret_cast<PUCHAR>(buffer),
            static_cast<ULONG>(T_size),
            0));
        lua_pushlstring(L, buffer, T_size);
      }

    private:
      alg_handle_t alg_;
      std::vector<UCHAR> hash_buffer_;
      hash_handle_t hash_;
    };
  }

  void open_cryptor() {}
  void open_hasher() {}

  cryptor* new_aes_cbc_encryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    if (iv_size != 16) {
      throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
    }
    return new_userdata<aes_encryptor_impl>(L, "brigid.cryptor", key_data, key_size, iv_data, std::move(ref));
  }

  cryptor* new_aes_128_cbc_encryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    return new_aes_cbc_encryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref));
  }

  cryptor* new_aes_192_cbc_encryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    return new_aes_cbc_encryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref));
  }

  cryptor* new_aes_256_cbc_encryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    return new_aes_cbc_encryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref));
  }

  cryptor* new_aes_cbc_decryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    if (iv_size != 16) {
      throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
    }
    return new_userdata<aes_decryptor_impl>(L, "brigid.cryptor", key_data, key_size, iv_data, std::move(ref));
  }

  cryptor* new_aes_128_cbc_decryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    return new_aes_cbc_decryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref));
  }

  cryptor* new_aes_192_cbc_decryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    return new_aes_cbc_decryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref));
  }

  cryptor* new_aes_256_cbc_decryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    return new_aes_cbc_decryptor(L, key_data, key_size, iv_data, iv_size, std::move(ref));
  }

  hasher* new_sha1_hasher(lua_State* L) {
    return new_userdata<hasher_impl<20> >(L, "brigid.hasher", BCRYPT_SHA1_ALGORITHM);
  }

  hasher* new_sha256_hasher(lua_State* L) {
    return new_userdata<hasher_impl<32> >(L, "brigid.hasher", BCRYPT_SHA256_ALGORITHM);
  }

  hasher* new_sha512_hasher(lua_State* L) {
    return new_userdata<hasher_impl<64> >(L, "brigid.hasher", BCRYPT_SHA512_ALGORITHM);
  }
}
