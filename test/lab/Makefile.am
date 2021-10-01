# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

noinst_PROGRAMS = \
	test_client.exe \
	test_server.exe

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
