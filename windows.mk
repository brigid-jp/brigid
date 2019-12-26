# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

CPPFLAGS = /I$(LUA_INCDIR) /I. /Iinclude /D_CRT_SECURE_NO_WARNINGS /DBRIGID_ZONE1=$(BRIGID_ZONE1)
CXXFLAGS = $(CFLAGS) /W3 /EHsc

OBJS = \
	src\lib\common_windows.obj \
	src\lib\crypto.obj \
	src\lib\crypto_windows.obj \
	src\lib\error.obj \
	src\lib\http.obj \
	src\lib\http_impl.obj \
	src\lib\http_windows.obj \
	src\lib\stdio.obj \
	src\lib\version.obj \
	src\lua\common.obj \
	src\lua\crypto.obj \
	src\lua\data.obj \
	src\lua\data_writer.obj \
	src\lua\file_writer.obj \
	src\lua\http.obj \
	src\lua\module.obj \
	src\lua\scope_exit.obj \
	src\lua\version.obj \
	src\lua\view.obj \
	src\lua\zone.obj
TARGET = brigid.dll

all: $(TARGET)

clean:
	del $(OBJS) $(TARGET)

check:
	cd test\lua && test.bat

install:
	xcopy "$(TARGET)" "$(LIBDIR)"

$(TARGET): $(OBJS)
	link $(LIBFLAG) /DEF:brigid.def $** "$(LUA_LIBDIR)\$(LUALIB)" bcrypt.lib winhttp.lib /OUT:$@

.cpp.obj:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) /c $< /Fo$@
