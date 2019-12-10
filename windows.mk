# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

CPPFLAGS = /I$(LUA_INCDIR) /I. /Iinclude /D_CRT_SECURE_NO_WARNINGS
CXXFLAGS = $(CFLAGS) /W3 /EHsc

OBJS = \
	src\lib\crypto.obj \
	src\lib\crypto_windows.obj \
	src\lib\http.obj \
	src\lib\http_impl.obj \
	src\lib\http_windows.obj \
	src\lib\util_windows.obj \
	src\lib\version.obj \
	src\lua\crypto.obj \
	src\lua\data.obj \
	src\lua\http.obj \
	src\lua\module.obj \
	src\lua\util_lua.obj \
	src\lua\version.obj \
	src\lua\view.obj
TARGET = brigid.dll

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

install:
	xcopy "$(TARGET)" "$(LIBDIR)"

$(TARGET): $(OBJS)
	link $(LIBFLAG) /DEF:brigid.def $** "$(LUA_LIBDIR)\$(LUALIB)" bcrypt.lib winhttp.lib /OUT:$@

.cpp.obj:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) /c $< /Fo$@
