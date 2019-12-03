# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

.SUFFIXES: .java .class

CLASSES = \
	jp/brigid/AESDecryptor.class \
	jp/brigid/AESEncryptor.class \
	jp/brigid/HttpSession.class

all: $(CLASSES)

clean:
	rm -f $(CLASSES)

.java.class:
	javac $<
