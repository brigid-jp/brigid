# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

luaexec_LTLIBRARIES = brigid.la

noinst_HEADERS = \
	common.hpp \
	common.lua \
	data.hpp \
	scope_exit.hpp \
	view.hpp

brigid_la_CPPFLAGS = -I$(top_srcdir)/include
brigid_la_LDFLAGS = -module -avoid-version -shared
brigid_la_LIBADD = $(top_srcdir)/src/lib/libbrigid.la
brigid_la_SOURCES = \
	common.cpp \
	crypto.cpp \
	data.cpp \
	data_writer.cpp \
	http.cpp \
	module.cpp \
	scope_exit.cpp \
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
