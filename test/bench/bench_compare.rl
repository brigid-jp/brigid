// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <stddef.h>
#include <string.h>
#include <chrono>
#include <iostream>

namespace brigid {
  namespace {
    const char* test_names[] = {
      "CLOCK_REALTIME",
      "CLOCK_REALTIME_COARSE",
      "CLOCK_MONOTONIC",
      "CLOCK_MONOTONIC_COARSE",
      "CLOCK_MONOTONIC_RAW",
      "CLOCK_MONOTONIC_RAW_APPROX",
      "CLOCK_BOOTTIME",
      "CLOCK_UPTIME_RAW",
      "CLOCK_UPTIME_RAW_APPROX",
      "std::chrono::system_clock",
      "std::chrono::steady_clock",
      "std::chrono::high_resolution_clock",
      "no such name",
    };

    %%{
      machine comparator;

      main :=
        ( /CLOCK_REALTIME/             @{ return 1; }
        | /CLOCK_REALTIME_COARSE/      @{ return 2; }
        | /CLOCK_MONOTONIC/            @{ return 3; }
        | /CLOCK_MONOTONIC_COARSE/     @{ return 4; }
        | /CLOCK_MONOTONIC_RAW/        @{ return 5; }
        | /CLOCK_MONOTONIC_RAW_APPROX/ @{ return 6; }
        | /CLOCK_BOOTTIME/             @{ return 7; }
        | /CLOCK_UPTIME_RAW/           @{ return 8; }
        | /CLOCK_UPTIME_RAW_APPROX/    @{ return 9; }
        );

      write data noerror nofinal noentry;
    }%%

    int test_compare_ragel(const char* name) {
      int cs = 0;

      %%write init;

      const char* p = name;
      const char* pe = nullptr;

      %%write exec;

      return 0;
    }

    int test_compare_pure(const char* name) {
      if (strcmp(name, "CLOCK_REALTIME") == 0) {
        return 1;
      } else if (strcmp(name, "CLOCK_REALTIME_COARSE") == 0) {
        return 2;
      } else if (strcmp(name, "CLOCK_MONOTONIC") == 0) {
        return 3;
      } else if (strcmp(name, "CLOCK_MONOTONIC_COARSE") == 0) {
        return 4;
      } else if (strcmp(name, "CLOCK_MONOTONIC_RAW") == 0) {
        return 5;
      } else if (strcmp(name, "CLOCK_MONOTONIC_RAW_APPROX") == 0) {
        return 6;
      } else if (strcmp(name, "CLOCK_BOOTTIME") == 0) {
        return 7;
      } else if (strcmp(name, "CLOCK_UPTIME_RAW") == 0) {
        return 8;
      } else if (strcmp(name, "CLOCK_UPTIME_RAW_APPROX") == 0) {
        return 9;
      } else {
        return 0;
      }
    }

    int x;
    void test() {
      x = 0;
      for (int i = 0; i < 1000000; ++i) {
        for (size_t j = 0; j < sizeof(test_names) / sizeof(test_names[0]); ++j ) {
          // x += test_compare_pure(test_names[j]);
          x += test_compare_ragel(test_names[j]);
        }
      }
    }

    void bench(int, char* []) {
      using clock_type = std::chrono::steady_clock;
      typename clock_type::time_point started;
      typename clock_type::time_point stopped;

      started = clock_type::now();
      test();
      stopped = clock_type::now();

      std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(stopped - started).count() << "\n";
    }
  }
}

int main(int ac, char* av[]) {
  brigid::bench(ac, av);
  return 0;
}
