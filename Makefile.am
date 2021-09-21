# Copyright (c) 2019,2020 <dev@brigid.jp>
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
	docs \
	include \
	m4/update.sh \
	src/java/java.mk \
	src/java/jp/brigid/*.java \
	test/cgi \
	windows.mk

dist-hook:
	rm -f `find $(distdir) -name '.*.swp'`
