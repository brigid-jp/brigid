# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

.SUFFIXES: .mm

CPPFLAGS =
CXXFLAGS = -Wall -W -O2 -std=c++11 -fobjc-arc
LDFLAGS = -framework Foundation

OBJS = \
	http_apple.o \
	main.o
TARGET = http.exe

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	c++ $(LDFLAGS) $^ -o $@

.cpp.o:
	c++ $(CPPFLAGS) $(CXXFLAGS) -c $<

.mm.o:
	c++ $(CPPFLAGS) $(CXXFLAGS) -c $<
