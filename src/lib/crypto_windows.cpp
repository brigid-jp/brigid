// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include <brigid/type_traits.hpp>

#include <windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace brigid {
  namespace {
    void check(NTSTATUS status) {
      if (!BCRYPT_SUCCESS(status)) {
        std::ostringstream out;
        out << "crypto_windows error number " << status;
        throw std::runtime_error(out.str());
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

    class aes_encryptor_impl : public encryptor_impl {
    public:
      aes_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t iv_size)
        : alg_(make_alg_handle()),
          key_(make_key_handle()),
          iv_(iv_size) {
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
        buffer_.resize(size);

        BCRYPT_KEY_HANDLE key = nullptr;
        check(BCryptGenerateSymmetricKey(
            alg_.get(),
            &key,
            buffer_.data(),
            static_cast<ULONG>(buffer_.size()),
            reinterpret_cast<PUCHAR>(const_cast<char*>(key_data)),
            static_cast<ULONG>(key_size),
            0));
        key_ = make_key_handle(key);

        std::copy(iv_data, iv_data + iv_size, iv_.begin());
      }

      virtual size_t block_bytes() const {
        return 16;
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        ULONG result = 0;
        check(BCryptEncrypt(
            key_.get(),
            reinterpret_cast<PUCHAR>(const_cast<char*>(in_data)),
            static_cast<ULONG>(in_size),
            nullptr,
            iv_.data(),
            static_cast<ULONG>(iv_.size()),
            reinterpret_cast<PUCHAR>(out_data),
            static_cast<ULONG>(out_size),
            &result,
            padding ? BCRYPT_BLOCK_PADDING : 0));
        return result;
      }

    private:
      alg_handle_t alg_;
      std::vector<UCHAR> buffer_;
      key_handle_t key_;
      std::vector<UCHAR> iv_;
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-128-cbc" || cipher == "aes-192-cbc" || cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new aes_encryptor_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
