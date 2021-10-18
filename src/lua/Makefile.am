# Copyright (c) 2019-2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

.rl.cxx:
	ragel -G2 $< -o $@

luaexec_LTLIBRARIES = brigid.la

noinst_HEADERS = \
	common.hpp \
	common.lua \
	data.hpp \
	scope_exit.hpp \
	stack_guard.hpp \
	thread_reference.hpp \
	view.hpp

brigid_la_CPPFLAGS = -I$(top_srcdir)/include
brigid_la_LDFLAGS = -module -avoid-version -shared
brigid_la_LIBADD = $(top_srcdir)/src/lib/libbrigid.la
brigid_la_SOURCES = \
	common.cpp \
	cryptor.cpp \
	data.cpp \
	data_writer.cpp \
	file_writer.cpp \
	hasher.cpp \
	http.cpp \
	json.cxx \
	module.cpp \
	scope_exit.cpp \
	stack_guard.cpp \
	thread_reference.cpp \
	version.cpp \
	view.cpp

if CRYPTO_OPENSSL
brigid_la_LDFLAGS += $(OPENSSL_LDFLAGS)
brigid_la_LIBADD += $(OPENSSL_LIBS)
endif

if HTTP_APPLE
brigid_la_LDFLAGS += -framework Foundation
else
if HTTP_CURL
brigid_la_LDFLAGS += $(CURL_LDFLAGS)
brigid_la_LIBADD += $(CURL_LIBS)
endif
endif

EXTRA_DIST = *.rl
