# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

noinst_LTLIBRARIES = libbrigid.la

noinst_HEADERS = \
	common_java.hpp \
	common_windows.hpp \
	http_impl.hpp

libbrigid_la_CPPFLAGS = -I$(top_srcdir)/include
libbrigid_la_LDFLAGS = -static
libbrigid_la_SOURCES = \
	error.cpp \
	http.cpp \
	http_impl.cpp \
	stdio.cpp

if HTTP_APPLE
libbrigid_la_SOURCES += http_apple.mm
else
if HTTP_CURL
libbrigid_la_CPPFLAGS += $(CURL_CFLAGS)
libbrigid_la_SOURCES += http_curl.cpp
endif
endif

EXTRA_DIST = \
	common_java.cpp \
	common_windows.cpp \
	http_java.cpp \
	http_windows.cpp \
	java.mk \
	windows.mk
