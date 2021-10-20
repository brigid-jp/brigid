// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include <brigid/stdio.hpp>

#include <stddef.h>
#include <exception>
#include <iostream>
#include <vector>

namespace brigid {
  namespace {
    %%{
      machine test_goto;

      prepush { stack.push_back(0); }
      postpop { stack.pop_back(); }

      value =
        ( "[" @{ fcall array; }
        | "\"" @{ ps = fpc + 1; fcall string; }
        );

      string :=
        ( "\"" @{ fret; }
        | alpha+
          ( "\"" @{ std::cout << "simple1(" << std::string(ps, fpc) << ")\n"; fret; }
          | "\\" @{ std::cout << "simple2(" << std::string(ps, fpc) << ")\n"; }
            alpha @{ std::cout << "escape1(" << fc << ")\n"; }
            ( alpha+ ${ std::cout << "_1(" << fc << ")\n"; }
            | "\\" alpha @{ std::cout << "escape2(" << fc << ")\n"; }
            )*
            "\"" @{ fret; }
          )
        | "\\" alpha @{ std::cout << "escape3(" << fc << ")\n"; }
          ( alpha+ ${ std::cout << "_2(" << fc << ")\n"; }
          | "\\" alpha @{ std::cout << "escape4(" << fc << ")\n"; }
          )*
          "\"" @{ fret; }
        );

      array :=
        ( "]" @{ std::cout << "]-----\narray0\n"; fret; }
        | value
          ( "," @{ std::cout << ",----\n"; }
            value
          )*
          "]" @{ std::cout << "]-----\narrayN\n"; fret; }
        );

      main := value "\n";

      write data noerror nofinal noentry;
    }%%

    void parse(const char* data, size_t size) {
      int cs = 0;
      int top = 0;
      const char* p = data;
      const char* pe = data + size;
      // const char* eof = pe;
      std::vector<int> stack;

      %%write init;

      const char* ps = nullptr;

      %%write exec;

      if (cs >= %%{ write first_final; }%% && stack.empty()) {
        std::cout << "ok\n";
        return;
      }

      std::cerr << "cannot parse at position " << (p - data + 1) << "\n";
    }
  }
}

int main(int ac, char* av[]) {
  try {
    if (ac < 1) {
      std::cout << "usage: " << av[0] << " path\n";
      return 1;
    }
    const char* path = av[1];
    brigid::file_handle_t handle = brigid::open_file_handle(path, "rb");

    std::vector<char> buffer;
    size_t p = 0;

    while (true) {
      buffer.resize(buffer.size() + 4096);
      size_t n = buffer.size() - p;
      size_t r = fread(&buffer[p], 1, n, handle.get());
      p += r;
      buffer.resize(p);
      if (r == 0) {
        break;
      }
    }
    brigid::parse(buffer.data(), buffer.size());

    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  } catch (...) {
    std::cerr << "unknown exception\n";
  }
  return 1;
}
