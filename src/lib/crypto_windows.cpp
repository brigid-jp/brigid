// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>

#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

#include <string.h>

#include <sstream>
#include <stdexcept>
#include <type_traits>
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

    using algorithm_provider_pointer_t = std::unique_ptr<typename std::remove_pointer<BCRYPT_ALG_HANDLE>::type, decltype(&close_algorithm_provider)>;
    using key_pointer_t = std::unique_ptr<typename std::remove_pointer<BCRYPT_KEY_HANDLE>::type, decltype(&BCryptDestroyKey)>;

    class aes_256_cbc_encryptor_impl : public encryptor_impl {
    public:
      aes_256_cbc_encryptor_impl(const char* key_data, size_t key_size, const char* iv_data, size_t)
        : alg_(nullptr, &close_algorithm_provider),
          key_(nullptr, &BCryptDestroyKey),
          iv_(16) {

        BCRYPT_ALG_HANDLE alg = nullptr;
        check(BCryptOpenAlgorithmProvider(&alg, BCRYPT_AES_ALGORITHM, nullptr, 0));
        alg_ = algorithm_provider_pointer_t(alg, &close_algorithm_provider);

        check(BCryptSetProperty(alg_.get(), BCRYPT_CHAINING_MODE, reinterpret_cast<PUCHAR>(BCRYPT_CHAIN_MODE_CBC), sizeof(BCRYPT_CHAIN_MODE_CBC), 0));

        DWORD size = 0;
        DWORD result = 0;

        check(BCryptGetProperty(alg_.get(), BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&size), sizeof(size), &result, 0));
        key_storage_.resize(size);

        BCRYPT_KEY_HANDLE key = nullptr;
        check(BCryptGenerateSymmetricKey(alg_.get(), &key, key_storage_.data(), key_storage_.size(), reinterpret_cast<PUCHAR>(const_cast<char*>(key_data)), key_size, 0));
        key_ = key_pointer_t(key, &BCryptDestroyKey);

        memmove(iv_.data(), iv_data, 16);
      }

      virtual size_t block_bytes() const {
        return 16;
      }

      virtual size_t update(const char* in_data, size_t in_size, char* out_data, size_t out_size, bool padding) {
        ULONG result = 0;
        check(BCryptEncrypt(
            key_.get(),
            reinterpret_cast<PUCHAR>(const_cast<char*>(in_data)),
            in_size,
            nullptr,
            iv_.data(),
            iv_.size(),
            reinterpret_cast<PUCHAR>(out_data),
            out_size,
            &result,
            padding ? BCRYPT_BLOCK_PADDING : 0));
        return result;
      }

    private:
      algorithm_provider_pointer_t alg_;
      std::vector<UCHAR> key_storage_;
      key_pointer_t key_;
      std::vector<UCHAR> iv_;
    };
  }

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string& cipher, const char* key_data, size_t key_size, const char* iv_data, size_t iv_size) {
    if (cipher == "aes-256-cbc") {
      return std::unique_ptr<encryptor_impl>(new aes_256_cbc_encryptor_impl(key_data, key_size, iv_data, iv_size));
    } else {
      throw std::runtime_error("unsupported cipher");
    }
  }
}
