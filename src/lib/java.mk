# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

JAVA_HOME = /Library/Java/JavaVirtualMachines/jdk1.8.0_131.jdk/Contents/Home
CPPFLAGS = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin -I../../include
CXXFLAGS = -Wall -W -O2 -std=c++11

OBJS = \
	crypto.o \
	crypto_impl.o \
	crypto_java.o
TARGET = libbrigid.a

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

$(TARGET): $(OBJS)
	ar cru $@ $^

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
