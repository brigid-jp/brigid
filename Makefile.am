ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src/lib test/lib

luaexec_LTLIBRARIES = brigid.la

brigid_la_LDFLAGS = -module -avoid-version -shared
brigid_la_CPPFLAGS =
brigid_la_LIBADD =
brigid_la_SOURCES = \
	common.cpp \
	module.cpp

if CRYPTO_OPENSSL
brigid_la_LDFLAGS += $(OPENSSL_LDFLAGS)
brigid_la_CPPFLAGS += $(OPENSSL_INCLUDES)
brigid_la_LIBADD += $(OPENSSL_LIBS)
brigid_la_SOURCES += crypto_openssl.cpp
else
if CRYPTO_APPLE
brigid_la_SOURCES += crypto_apple.cpp
endif
endif
