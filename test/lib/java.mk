# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

JAVA_HOME = /Library/Java/JavaVirtualMachines/jdk1.8.0_131.jdk/Contents/Home
CPPFLAGS = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin -I../../include -I../../src/lib
CXXFLAGS = -Wall -W -O2 -std=c++11
LDFLAGS = -shared

OBJS = \
	test.o \
	test_crypto.o \
	test_java.o \
	test_http.o \
	test_http_impl.o \
	test_version.o
TARGET = libjavatest.dylib

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

libjavatest.dylib: JavaTest.h $(OBJS) ../../src/lib/libbrigid.a
	$(CXX) $(LDFLAGS) $(OBJS) ../../src/lib/libbrigid.a -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
