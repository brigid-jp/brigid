# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

JAVA_HOME = $(shell ../../java_home.sh)
UNAME = $(shell uname | tr [:upper:] [:lower:])

CFLAGS = $(shell luarocks config variables.CFLAGS)
LUA_INCDIR = $(shell luarocks config variables.LUA_INCDIR)
LUA_LIBDIR = $(shell luarocks config variables.LUA_LIBDIR)

CPPFLAGS = "-I$(LUA_INCDIR)" "-I$(JAVA_HOME)/include" "-I$(JAVA_HOME)/include/$(UNAME)" -I../..
CXXFLAGS = -Wall -W -Wno-missing-field-initializers -std=c++11 $(CFLAGS)
LDFLAGS = -shared "-L$(LUA_LIBDIR)" "-Wl,-rpath,$(LUA_LIBDIR)"

ifeq ($(UNAME),darwin)
	TARGET_SUFFIX = .dylib
else
	TARGET_SUFFIX = .so
endif

OBJS = test_java.o
TARGET = libjavatest$(TARGET_SUFFIX)

all: all-recursive $(TARGET)

all-recursive:
	(cd ../../src/lua && $(MAKE) -f java.mk all)

clean:
	rm -f $(OBJS) JavaTest.h JavaTest.class $(TARGET)
	(cd ../../src/lua && $(MAKE) -f java.mk clean)

check:
	java -classpath ../../src/java:. -Djava.library.path=. -Xcheck:jni JavaTest test.lua

JavaTest.h: JavaTest.java
	javac -h . JavaTest.java

$(TARGET): JavaTest.h $(OBJS) ../../src/lua/libbrigid.a
	$(CXX) $(LDFLAGS) $(OBJS) ../../src/lua/libbrigid.a -o $@ -llua

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
