# Copyright (c) 2019-2022,2024 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

.rl.cxx:
	ragel -G2 $< -o $@

luaexec_LTLIBRARIES = brigid.la

noinst_HEADERS = \
	common.hpp \
	common.lua \
	common_java.hpp \
	common_windows.hpp \
	crypto.hpp \
	data.hpp \
	dir_windows.hpp \
	dir_unix.hpp \
	error.hpp \
	function.hpp \
	http.hpp \
	http_impl.hpp \
	module.lua \
	noncopyable.hpp \
	scope_exit.hpp \
	stack_guard.hpp \
	stdio.hpp \
	stopwatch.hpp \
	thread_reference.hpp \
	type_traits.hpp \
	view.hpp \
	writer.hpp

brigid_la_CPPFLAGS = -I$(top_srcdir)/include
brigid_la_LDFLAGS = -module -avoid-version -shared
brigid_la_LIBADD =
brigid_la_SOURCES = \
	common.cpp \
	crypto.cpp \
	cryptor.cpp \
	data.cpp \
	data_writer.cpp \
	dir.cpp \
	error.cpp \
	file_writer.cpp \
	function.cpp \
	hasher.cxx \
	http.cpp \
	http_impl.cpp \
	json.cpp \
	json_parse.cxx \
	module.cpp \
	new_decryptor.cxx \
	new_encryptor.cxx \
	scope_exit.cpp \
	stack_guard.cpp \
	stdio.cpp \
	stopwatch.cxx \
	stopwatch_unix.cxx \
	thread_reference.cpp \
	view.cpp \
	write_json_string.cxx \
	write_urlencoded.cxx \
	writer.cpp

if CRYPTO_APPLE
brigid_la_SOURCES += crypto_apple.cpp
else
if CRYPTO_OPENSSL
brigid_la_CPPFLAGS += $(OPENSSL_INCLUDES)
brigid_la_LDFLAGS += $(OPENSSL_LDFLAGS)
brigid_la_LIBADD += $(OPENSSL_LIBS)
brigid_la_SOURCES += crypto_openssl.cpp
endif
endif

if HTTP_APPLE
brigid_la_LDFLAGS += -framework Foundation
brigid_la_SOURCES += http_apple.mm
else
if HTTP_CURL
brigid_la_CPPFLAGS += $(CURL_CFLAGS)
brigid_la_LDFLAGS += $(CURL_LDFLAGS)
brigid_la_LIBADD += $(CURL_LIBS)
brigid_la_SOURCES += http_curl.cpp
endif
endif

if HAVE_LD_VERSION_SCRIPT
brigid_la_LDFLAGS += -Wl,--version-script=module.map
endif

EXTRA_DIST = \
	*.rl \
	common_java.cpp \
	common_windows.cpp \
	crypto_java.cpp \
	crypto_windows.cpp \
	http_java.cpp \
	http_windows.cpp \
	java.mk \
	module.map \
	stopwatch_windows.cpp
