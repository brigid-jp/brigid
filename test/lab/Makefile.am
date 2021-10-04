# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

.rl.cxx:
	ragel -G2 $< -o $@

noinst_PROGRAMS = \
	test_client.exe \
	test_server.exe \
	test_parser.exe

test_client_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_client_exe_LDFLAGS =
test_client_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_client_exe_SOURCES = \
	test_client.cpp

test_server_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_server_exe_LDFLAGS =
test_server_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_server_exe_SOURCES = \
	test_server.cpp \
	websocket_server_parser.cxx

test_parser_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_parser_exe_LDFLAGS =
test_parser_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_parser_exe_SOURCES = \
	test_parser.cpp \
	websocket_server_parser.cxx

EXTRA_DIST = \
	test_common.hpp \
	websocket_server_parser.rl