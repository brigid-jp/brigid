# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

TESTS = bench.sh

check_PROGRAMS = \
	bench_compare.exe

bench_compare_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
bench_compare_exe_LDFLAGS =
bench_compare_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
bench_compare_exe_SOURCES = \
	bench_compare.cpp

EXTRA_DIST = \
	*.lua \
	bench.bat \
	bench.sh
