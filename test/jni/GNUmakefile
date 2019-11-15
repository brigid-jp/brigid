# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

JAVA_HOME = /Library/Java/JavaVirtualMachines/jdk1.8.0_131.jdk/Contents/Home
CPPFLAGS = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin
CXXFLAGS = -Wall -W -O2 -std=c++11
LDFLAGS = -dynamiclib

OBJS = \
	test.o
TARGET = test.dylib

all:: $(TARGET)

clean::
	rm -f *.o Test.h Test.class $(TARGET)

check::
	java -Djava.library.path=. Test

Test.class: Test.java
	javac -h . Test.java

test.dylib: $(OBJS) Test.class
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
