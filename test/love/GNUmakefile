# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

TARGET = brigid-version-test.love

all: $(TARGET)

clean:
	rm -f brigid-*-test.love

$(TARGET): $(wildcard *.lua)
	zip -r9 $@ *.lua
