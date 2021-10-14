# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

abnf.rl: abnf.lua errata.txt
	lua abnf.lua

.rl.cxx:
	ragel -G2 $< -o $@

check_PROGRAMS = \
	test_client.exe \
	test_server.exe \
	test_parser.exe \
	test_websocket_server.exe

test_client_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_client_exe_LDFLAGS =
test_client_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_client_exe_SOURCES = \
	test_client.cpp

test_server_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_server_exe_LDFLAGS =
test_server_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_server_exe_SOURCES = \
	test_server.cpp

test_parser_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_parser_exe_LDFLAGS =
test_parser_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_parser_exe_SOURCES = \
	test_parser.cpp \
	abnf.cxx \
	http_request_parser.cxx \
	websocket_frame_parser.cxx

test_websocket_server_exe_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/lib
test_websocket_server_exe_LDFLAGS =
test_websocket_server_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_websocket_server_exe_SOURCES = \
	test_websocket_server.cpp \
	abnf.cxx \
	http_request_parser.cxx

if CRYPTO_OPENSSL
test_websocket_server_exe_LDFLAGS += $(OPENSSL_LDFLAGS)
test_websocket_server_exe_LDADD += $(OPENSSL_LIBS)
endif

EXTRA_DIST = \
	*.lua \
	*.rl \
	*.txt \
	test_common.hpp \
	http_request_parser.hpp \
	websocket_frame_parser.hpp \
	windows.mk
