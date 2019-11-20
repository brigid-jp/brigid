# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

OBJS = \
	src\lib\crypto.obj \
	src\lib\crypto_impl.obj \
	src\lib\crypto_windows.obj \
	src\lua\common.obj \
	src\lua\crypto.obj \
	src\lua\module.obj
TARGET = brigid.dll

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	link $(LIBFLAG) /DEF:brigid.def $** "$(LUA_LIBDIR)\$(LUALIB)" bcrypt.lib /OUT:$@

.cpp.obj:
	$(CC) $(CFLAGS) /EHsc /I$(LUA_INCDIR) /Iinclude /c $< /Fo$@

install:
	xcopy "$(TARGET)" "$(LIBDIR)"
