luaexec_LTLIBRARIES = brigid_core.la

brigid_core_la_LDFLAGS = -module -avoid-version -shared
brigid_core_la_CPPFLAGS =
brigid_core_la_LIBADD =
brigid_core_la_SOURCES = \
	common.cpp \
	module.cpp

if CRYPTO_OPENSSL
brigid_core_la_LDFLAGS += $(OPENSSL_LDFLAGS)
brigid_core_la_CPPFLAGS += $(OPENSSL_INCLUDES)
brigid_core_la_LIBADD += $(OPENSSL_LIBS)
brigid_core_la_SOURCES += crypto_openssl.cpp
else
if CRYPTO_APPLE
brigid_core_la_SOURCES += crypto_apple.cpp
endif
endif
