# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	common.obj \
	module.obj \
	crypto_windows.obj
TARGET = brigid.dll

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	link $(LIBFLAG) /DEF:brigid.def /OUT:$@ $(LUA_LIBDIR)\$(LUALIB) $**

.cpp.obj:
	$(CC) $(CFLAGS) /I$(LUA_INCDIR) /c $<

install:
	xcopy "$(TARGET)" "$(LIBDIR)"
