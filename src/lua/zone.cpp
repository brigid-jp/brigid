// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "brigid_zone.hpp"

#define BRIGID_ZONE_INITIALIZE(v, i) \
  do { \
    zone[i + 0] = static_cast<uint8_t>(0xFF & (v) >> 24); \
    zone[i + 1] = static_cast<uint8_t>(0xFF & (v) >> 16); \
    zone[i + 2] = static_cast<uint8_t>(0xFF & (v) >> 8); \
    zone[i + 3] = static_cast<uint8_t>(0xFF & (v)); \
  } while (false)

namespace brigid {
  namespace {
    static const int zone_size = 32;
    uint8_t zone[zone_size];

    void impl_put(lua_State* L) {
      lua_Integer position = luaL_checkinteger(L, 1);
      lua_Integer value = luaL_checkinteger(L, 2);
      if (position < 1 || position > zone_size) {
        throw BRIGID_LOGIC_ERROR("invalid position");
      }
      if (value < 0 || value > 255) {
        throw BRIGID_LOGIC_ERROR("invalid value");
      }
      zone[position - 1] = value;
    }

    void impl_dump(lua_State*) {
      for (size_t i = 0; i < zone_size; i += 8) {
        printf("%02X", zone[i]);
        for (size_t j = i + 1; j < i + 8; ++j) {
          printf(" %02X", zone[j]);
        }
        printf("\n");
      }
    }
  }

  void initialize_zone(lua_State* L) {
#if BRIGID_ZONE1+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE1, 0);
#endif
#if BRIGID_ZONE2+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE2, 4);
#endif
#if BRIGID_ZONE3+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE3, 8);
#endif
#if BRIGID_ZONE4+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE4, 12);
#endif
#if BRIGID_ZONE5+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE5, 16);
#endif
#if BRIGID_ZONE6+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE6, 20);
#endif
#if BRIGID_ZONE7+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE7, 24);
#endif
#if BRIGID_ZONE8+0
    BRIGID_ZONE_INITIALIZE(BRIGID_ZONE8, 28);
#endif

    lua_newtable(L);
    {
      set_field(L, -1, "put", impl_put);
      set_field(L, -1, "dump", impl_dump);
    }
    set_field(L, -2, "zone");
  }
}
