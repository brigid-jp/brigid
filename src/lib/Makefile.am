# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

noinst_LTLIBRARIES = libbrigid.la

noinst_HEADERS = \
	common_java.hpp \
	common_windows.hpp

libbrigid_la_CPPFLAGS = -I$(top_srcdir)/include
libbrigid_la_LDFLAGS = -static
libbrigid_la_SOURCES = \
	error.cpp \
	stdio.cpp

EXTRA_DIST = \
	common_java.cpp \
	common_windows.cpp \
	java.mk \
	windows.mk
