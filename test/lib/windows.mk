# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

CPPFLAGS = /I..\..\include /I..\..\src\lib /D_CRT_SECURE_NO_WARNINGS
CXXFLAGS = /nologo /MD /O2 /W3 /EHsc

OBJS = \
	test.obj \
	test_crypto.obj \
	test_http.obj \
	test_http_impl.obj \
	test_main.obj \
	test_version.obj

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
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $** bcrypt.lib winhttp.lib /Fe$@

.cpp.obj:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) /c $< /Fo$@
