#ifndef BRIGID_VERSION_HPP
#define BRIGID_VERSION_HPP

#include <brigid/version.hpp>

namespace brigid {
  const char* get_version() {
#define m4_define(_, value) return #value;
#include "version.m4"
#undef m4_define
  }
}

#endif
