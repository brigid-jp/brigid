luaexec_LTLIBRARIES = brigid_core.la

brigid_core_la_LDFLAGS = -module -avoid-version -shared
brigid_core_la_SOURCES = module.cpp

if HAVE_OPENSSL
brigid_core_la_SOURCES += evp.cpp
endif
