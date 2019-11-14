TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
	test_encryptor.exe

test_encryptor_exe_CPPFLAGS = -I$(top_srcdir)/include
test_encryptor_exe_LDADD = $(top_srcdir)/src/lib/libbrigid.la
test_encryptor_exe_SOURCES = test_encryptor.cpp
