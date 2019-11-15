# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

TESTS = \
	test_encryptor.exe

all: $(TESTS)

clean:
	del $(TESTS)

.cpp.exe:
	$(CC) /MD /O2 /EHsc /I..\..\include $< ..\..\src\lib\brigid.lib
