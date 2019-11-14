// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_CRYPTO_HPP
#define BRIGID_CRYPTO_HPP

#include <cstddef>
#include <memory>
#include <string>

namespace brigid {
  class encryptor_impl {
  public:
    virtual ~encryptor_impl() = 0;
    virtual std::size_t key_size() const = 0;
    virtual std::size_t block_size() const = 0;
    virtual void open(const char*, std::size_t, const char*, std::size_t) = 0;
  };

  std::unique_ptr<encryptor_impl> make_encryptor_impl(const std::string&);

  class encryptor {
  public:
    encryptor(const std::string&, const char*, std::size_t, const char*, std::size_t);
    std::size_t key_size() const;
    std::size_t block_size() const;
  private:
    std::unique_ptr<encryptor_impl> impl_;
  };
}

#endif
