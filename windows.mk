# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	src/lib/crypto.obj \
	src/lib/crypto_windows.obj \
	common.obj \
	module.obj \
	crypto_windows.obj
TARGET = brigid.dll

all: $(TARGET)

$(TARGET): $(OBJS)
	link $(LIBFLAG) /DEF:brigid.def /OUT:$@ $(LUA_LIBDIR)\$(LUALIB) $**

.cpp.obj:
	$(CC) $(CFLAGS) /I$(LUA_INCDIR) /Iinclude /c $< /Fo$@

install:
	xcopy "$(TARGET)" "$(LIBDIR)"
