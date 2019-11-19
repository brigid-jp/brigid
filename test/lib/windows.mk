# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	test.obj \
	test_encryptor.obj \
	test_main.obj

TARGET = test.exe

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

check:
	$(TARGET)

$(TARGET): $(OBJS) ..\..\src\lib\brigid.lib
	$(CC) /MD /O2 /EHsc /I..\..\include $** bcrypt.lib /Fo$@

.cpp.obj:
	$(CC) /MD /O2 /W3 /EHsc /I..\..\include /c $<
