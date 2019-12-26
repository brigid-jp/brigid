// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/error.hpp>
#include "common.hpp"

#include <lua.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace brigid {
  namespace {
    static const int zone_size = 32;
    uint8_t zone[zone_size];

    void impl_put(lua_State* L) {
      int position = luaL_checkinteger(L, 1);
      int value = luaL_checkinteger(L, 2);
      if (position < 1 || position > zone_size) {
        throw BRIGID_LOGIC_ERROR("invalid position");
      }
      if (value < 0 || value > 255) {
        throw BRIGID_LOGIC_ERROR("invalid value");
      }
      zone[position - 1] = value;
    }

    void impl_dump(lua_State* L) {
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
    zone[0] = BRIGID_ZONE1 >> 24;
    zone[1] = BRIGID_ZONE1 >> 16;
    zone[2] = BRIGID_ZONE1 >> 8;
    zone[3] = BRIGID_ZONE1;
#else
    zone[0] = 0xFE;
    zone[1] = 0xED;
    zone[2] = 0xFA;
    zone[3] = 0xCE;
#endif

    lua_newtable(L);
    {
      set_field(L, -1, "put", impl_put);
      set_field(L, -1, "dump", impl_dump);
    }
    set_field(L, -2, "zone");
  }
}
