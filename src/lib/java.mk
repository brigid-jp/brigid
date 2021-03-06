# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

JAVA_HOME = /Library/Java/JavaVirtualMachines/jdk1.8.0_131.jdk/Contents/Home
CPPFLAGS = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin -I../.. -I../../include
CXXFLAGS = -Wall -W -O2 -std=c++11

OBJS = \
	common_java.o \
	crypto.o \
	crypto_java.o \
	error.o \
	http.o \
	http_impl.o \
	http_java.o \
	stdio.o \
	version.o
TARGET = libbrigid.a

all: all-recursive $(TARGET)

all-recursive:
	(cd ../java && $(MAKE) -f java.mk all)

clean:
	rm -f $(OBJS) $(TARGET)
	(cd ../java && $(MAKE) -f java.mk clean)

$(TARGET): $(OBJS)
	ar cru $@ $^

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
