# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src/lib src/lua test/lib test/lua

EXTRA_DIST = \
	.gitignore \
	LICENSE \
	README.md \
	brigid-*.rockspec \
	brigid.def \
	brigid.hpp \
	brigid_zone.hpp \
	brigid_zone.lua \
	docs \
	include \
	m4/update.sh \
	src/java/java.mk \
	src/java/jp/brigid/*.java \
	test/cgi \
	test/love \
	windows.mk

dist-hook:
	rm -f `find $(distdir) -name '.*.swp'`
