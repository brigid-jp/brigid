# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	crypto.obj \
	crypto_impl.obj \
	crypto_windows.obj \
	http.obj \
	http_windows.obj
TARGET = brigid.lib

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	lib $** /OUT:$@

.cpp.obj:
	$(CC) /MD /O2 /W3 /EHsc /I..\..\include /c $< /Fo$@
