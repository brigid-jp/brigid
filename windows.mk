# Copyright (c) 2019-2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

CPPFLAGS = /I$(LUA_INCDIR) /I. /Iinclude /D_CRT_SECURE_NO_WARNINGS
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
	src\lua\cryptor.obj \
	src\lua\data.obj \
	src\lua\data_writer.obj \
	src\lua\file_writer.obj \
	src\lua\function.obj \
	src\lua\hasher.obj \
	src\lua\http.obj \
	src\lua\json.obj \
	src\lua\json_parse.obj \
	src\lua\module.obj \
	src\lua\scope_exit.obj \
	src\lua\stack_guard.obj \
	src\lua\thread_reference.obj \
	src\lua\stopwatch.obj \
	src\lua\stopwatch_windows.obj \
	src\lua\view.obj
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

.cxx.obj:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) /c $< /Fo$@
