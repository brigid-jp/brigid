// Copyright (c) 2019,2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/crypto.hpp>
#include "test.hpp"

int main(int ac, char* av[]) {
  brigid::open_cryptor();
  brigid::open_hasher();
  return brigid::run_test_cases(ac, av);
}
