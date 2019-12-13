# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src/lib src/lua test/lib test/lua

EXTRA_DIST = \
	docs \
	m4/update.sh

dist-hook:
	rm -f `find $(distdir) -name '.*.swp'`
