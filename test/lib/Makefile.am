# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
	test.exe

test_exe_CPPFLAGS = -I$(top_srcdir)/include
test_exe_LDFLAGS =
test_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_exe_SOURCES = \
	test.cpp \
	test_crypto.cpp \
	test_main.cpp

if CRYPTO_OPENSSL
test_exe_LDFLAGS += $(OPENSSL_LDFLAGS)
test_exe_LDADD += $(OPENSSL_LIBS)
endif

if HTTP_APPLE
test_exe_LDFLAGS += -framework Foundation
endif
