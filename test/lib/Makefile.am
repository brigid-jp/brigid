# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
	test_encryptor.exe

test_encryptor_exe_CPPFLAGS = -I$(top_srcdir)/include
test_encryptor_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_encryptor_exe_SOURCES = test_encryptor.cpp

if CRYPTO_OPENSSL
test_encryptor_exe_LDADD += $(OPENSSL_LIBS)
endif
