# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

JAVA_HOME = /Library/Java/JavaVirtualMachines/jdk1.8.0_131.jdk/Contents/Home
CPPFLAGS = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin
CXXFLAGS = -Wall -W -O2 -std=c++11
LDFLAGS = -shared

OBJS = \
	test.o
TARGET = libtest.dylib

all:: $(TARGET)

clean::
	rm -f *.o Test.h Test.class $(TARGET)

check::
	java -Djava.library.path=. Test

Test.h: Test.java
	javac -h . Test.java

libtest.dylib: Test.h $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
