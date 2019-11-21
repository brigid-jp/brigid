# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	http_windows.obj \
	main.obj
TARGET = http.exe

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) /MD /O2 /W3 /EHsc $** /Fe:$@

.cpp.obj:
	$(CC) /MD /O2 /W3 /EHsc /c $< /Fo:$@
