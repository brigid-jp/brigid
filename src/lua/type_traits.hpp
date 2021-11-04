// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_TYPE_TRAITS_HPP
#define BRIGID_TYPE_TRAITS_HPP

#include <type_traits>

namespace brigid {
  template <bool T_condition, class T = void>
  using enable_if_t = typename std::enable_if<T_condition, T>::type;

  template <class T>
  using remove_pointer_t = typename std::remove_pointer<T>::type;
}

#endif
