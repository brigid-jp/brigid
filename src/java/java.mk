# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

.SUFFIXES: .java .class

CLASSES = \
	jp/brigid/AESCryptor.class \
	jp/brigid/Hasher.class \
	jp/brigid/HttpAuthenticator.class \
	jp/brigid/HttpTask.class

all: $(CLASSES)

clean:
	rm -f $(CLASSES)

.java.class:
	javac $<
