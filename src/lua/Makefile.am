# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

luaexec_LTLIBRARIES = brigid.la

noinst_HEADERS = \
	common.hpp \
	common.lua \
	cryptor.hpp \
	data.hpp \
	hasher.hpp \
	scope_exit.hpp \
	view.hpp

brigid_la_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-DBRIGID_ZONE1=$(BRIGID_ZONE1) \
	-DBRIGID_ZONE2=$(BRIGID_ZONE2) \
	-DBRIGID_ZONE3=$(BRIGID_ZONE3) \
	-DBRIGID_ZONE4=$(BRIGID_ZONE4) \
	-DBRIGID_ZONE5=$(BRIGID_ZONE5) \
	-DBRIGID_ZONE6=$(BRIGID_ZONE6) \
	-DBRIGID_ZONE7=$(BRIGID_ZONE7) \
	-DBRIGID_ZONE8=$(BRIGID_ZONE8)
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
	module.cpp \
	scope_exit.cpp \
	version.cpp \
	view.cpp \
	zone.cpp

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
