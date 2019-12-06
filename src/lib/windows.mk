# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

CPPFLAGS = /I..\.. /I..\..\include /D_CRT_SECURE_NO_WARNINGS
CXXFLAGS = /nologo /MD /O2 /W3 /EHsc

OBJS = \
	crypto.obj \
	crypto_windows.obj \
	http.obj \
	http_impl.obj \
	http_windows.obj \
	util_windows.obj \
	version.obj
TARGET = brigid.lib

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	lib $** /OUT:$@

.cpp.obj:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) /c $< /Fo$@
