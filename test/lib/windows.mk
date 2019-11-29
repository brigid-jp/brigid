# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	test.obj \
	test_crypto.obj \
	test_http.obj \
	test_http_impl.obj \
	test_main.obj

TARGET = test.exe

all: all-recursive $(TARGET)

all-recursive:
	cd ..\..\src\lib && $(MAKE) /f windows.mk all

clean:
	del $(OBJS) $(TARGET)
	cd ..\..\src\lib && $(MAKE) /f windows.mk clean

check:
	$(TARGET)

$(TARGET): $(OBJS) ..\..\src\lib\brigid.lib
	$(CC) /MD /O2 /W3 /EHsc /I..\..\include /I..\..\src\lib $** bcrypt.lib winhttp.lib /Fe$@

.cpp.obj:
	$(CC) /MD /O2 /W3 /EHsc /I..\..\include /I..\..\src\lib /c $< /Fo$@
