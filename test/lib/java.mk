# Copyright (c) 2019,2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

UNAME = $(shell uname | tr [:upper:] [:lower:])
ifeq ($(UNAME),darwin)
	TARGET_SUFFIX = .dylib
endif
ifeq ($(UNAME),linux)
	TARGET_SUFFIX = .so
endif

CPPFLAGS = "-I$(JAVA_HOME)/include" "-I$(JAVA_HOME)/include/$(UNAME)" -I../../include -I../../src/lib
CXXFLAGS = -Wall -W -Wno-missing-field-initializers -O2 -std=c++11 -fPIC
LDFLAGS = -shared

OBJS = \
	test.o \
	test_crypto.o \
	test_java.o \
	test_http.o \
	test_http_impl.o \
	test_version.o
TARGET = libjavatest$(TARGET_SUFFIX)

all: all-recursive $(TARGET)

all-recursive:
	(cd ../../src/lib && $(MAKE) -f java.mk all)

clean:
	rm -f $(OBJS) JavaTest.h JavaTest.class $(TARGET)
	(cd ../../src/lib && $(MAKE) -f java.mk clean)

check:
	java -classpath ../../src/java:. -Djava.library.path=. -Xcheck:jni JavaTest

JavaTest.h: JavaTest.java
	javac -h . JavaTest.java

$(TARGET): JavaTest.h $(OBJS) ../../src/lib/libbrigid.a
	$(CXX) $(LDFLAGS) $(OBJS) ../../src/lib/libbrigid.a -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
