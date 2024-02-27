# Copyright (c) 2019-2021,2024 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src/lua test/lua

EXTRA_DIST = \
	.gitignore \
	LICENSE \
	README.md \
	brigid-*.rockspec \
	brigid.def \
	brigid.hpp \
	brigid_version.txt \
	docs \
	java_home.sh \
	m4/update.sh \
	src/java/java.mk \
	src/java/jp/brigid/*.java \
	test/cgi \
	windows.mk

dist-hook:
	rm -f `find $(distdir) -name '.*.swp'`
