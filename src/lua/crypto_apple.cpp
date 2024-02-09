// Copyright (c) 2021,2022,2024 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "common.hpp"
#include "crypto.hpp"
#include "error.hpp"
#include "noncopyable.hpp"
#include "type_traits.hpp"

#include <lua.hpp>

#include <CommonCrypto/CommonCrypto.h>

#include <stddef.h>
#include <utility>
#include <memory>

namespace brigid {
  namespace {
    const char* get_error_message(CCCryptorStatus code) {
      switch (code) {
        case -4300: return "kCCParamError";
        case -4301: return "kCCBufferTooSmall";
        case -4302: return "kCCMemoryFailure";
        case -4303: return "kCCAlignmentError";
        case -4304: return "kCCDecodeError";
        case -4305: return "kCCUnimplemented";
        case -4306: return "kCCOverflow";
        case -4307: return "kCCRNGFailure";
        case -4308: return "kCCUnspecifiedError";
        case -4309: return "kCCCallSequenceError";
        case -4310: return "kCCKeySizeError";
        case -4311: return "kCCInvalidKey";
      }
      return nullptr;
    }

    void check(CCCryptorStatus code) {
      if (code != kCCSuccess) {
        if (const char* message = get_error_message(code)) {
          throw BRIGID_RUNTIME_ERROR(message, make_error_code("CommonCrypto error", code));
        } else {
          throw BRIGID_RUNTIME_ERROR(make_error_code("CommonCrypto error", code));
        }
      }
    }

    using cryptor_ref_t = std::unique_ptr<remove_pointer_t<CCCryptorRef>, decltype(&CCCryptorRelease)>;

    cryptor_ref_t make_cryptor_ref(CCCryptorRef cryptor = nullptr) {
      return cryptor_ref_t(cryptor, &CCCryptorRelease);
    }

    class aes_cryptor_impl : public cryptor, private noncopyable {
    public:
      aes_cryptor_impl(CCOperation operation, const char* key_data, size_t key_size, const char* iv_data, size_t buffer_size, thread_reference&& ref)
        : cryptor(std::move(ref)),
          cryptor_(make_cryptor_ref()),
          buffer_size_(buffer_size) {
        CCCryptorRef cryptor = nullptr;
        check(CCCryptorCreateWithMode(operation, kCCModeCBC, kCCAlgorithmAES, kCCOptionPKCS7Padding, iv_data, key_data, key_size, nullptr, 0, 0, 0, &cryptor));
        cryptor_ = make_cryptor_ref(cryptor);
      }

      virtual size_t impl_calculate_buffer_size(size_t in_size) const {
        return in_size + buffer_size_;
      };

      virtual size_t impl_update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        size_t size1 = 0;
        size_t size2 = 0;
        check(CCCryptorUpdate(cryptor_.get(), in_data, in_size, out_data, out_size, &size1));
        if (padding) {
          check(CCCryptorFinal(cryptor_.get(), out_data + size1, out_size - size1, &size2));
        }
        return size1 + size2;
      }

      virtual void impl_close() {
        cryptor_ = make_cryptor_ref();
      }

    private:
      cryptor_ref_t cryptor_;
      size_t buffer_size_;
    };

    class sha1_hasher_impl : public hasher, private noncopyable {
    public:
      sha1_hasher_impl()
        : ctx_() {
        CC_SHA1_Init(&ctx_);
      }

      virtual void update(const char* data, size_t size) {
        CC_SHA1_Update(&ctx_, data, size);
      }

      virtual void digest(lua_State* L) {
        char buffer[CC_SHA1_DIGEST_LENGTH] = {};
        CC_SHA1_Final(reinterpret_cast<unsigned char*>(buffer), &ctx_);
        lua_pushlstring(L, buffer, CC_SHA1_DIGEST_LENGTH);
      }

    private:
      CC_SHA1_CTX ctx_;
    };

    class sha256_hasher_impl : public hasher, private noncopyable {
    public:
      sha256_hasher_impl()
        : ctx_() {
        CC_SHA256_Init(&ctx_);
      }

      virtual void update(const char* data, size_t size) {
        CC_SHA256_Update(&ctx_, data, size);
      }

      virtual void digest(lua_State* L) {
        char buffer[CC_SHA256_DIGEST_LENGTH] = {};
        CC_SHA256_Final(reinterpret_cast<unsigned char*>(buffer), &ctx_);
        lua_pushlstring(L, buffer, CC_SHA256_DIGEST_LENGTH);
      }

    private:
      CC_SHA256_CTX ctx_;
    };

    class sha512_hasher_impl : public hasher, private noncopyable {
    public:
      sha512_hasher_impl()
        : ctx_() {
        CC_SHA512_Init(&ctx_);
      }

      virtual void update(const char* data, size_t size) {
        CC_SHA512_Update(&ctx_, data, size);
      }

      virtual void digest(lua_State* L) {
        char buffer[CC_SHA512_DIGEST_LENGTH] = {};
        CC_SHA512_Final(reinterpret_cast<unsigned char*>(buffer), &ctx_);
        lua_pushlstring(L, buffer, CC_SHA512_DIGEST_LENGTH);
      }

    private:
      CC_SHA512_CTX ctx_;
    };
  }

  void open_cryptor() {}
  void open_hasher() {}

  cryptor* new_aes_cbc_encryptor(lua_State* L, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size, thread_reference&& ref) {
    if (iv_size != 16) {
      throw BRIGID_LOGIC_ERROR("invalid initialization vector size");
    }
    return new_userdata<aes_cryptor_impl>(L, "brigid.cryptor", kCCEncrypt, key_data, key_size, iv_data, 16, std::move(ref));
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
    return new_userdata<aes_cryptor_impl>(L, "brigid.cryptor", kCCDecrypt, key_data, key_size, iv_data, 0, std::move(ref));
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
    return new_userdata<sha1_hasher_impl>(L, "brigid.hasher");
  }

  hasher* new_sha256_hasher(lua_State* L) {
    return new_userdata<sha256_hasher_impl>(L, "brigid.hasher");
  }

  hasher* new_sha512_hasher(lua_State* L) {
    return new_userdata<sha512_hasher_impl>(L, "brigid.hasher");
  }
}
