# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

UNAME = $(shell uname | tr [:upper:] [:lower:])
CPPFLAGS = "-I$(JAVA_HOME)/include" "-I$(JAVA_HOME)/include/$(UNAME)" -I../.. -I../../include
CXXFLAGS = -Wall -W -Wno-missing-field-initializers -O2 -std=c++11 -fPIC

OBJS = \
	common.o \
	crypto.o \
	crypto_java.o \
	cryptor.o \
	data.o \
	data_writer.o \
	error.o \
	file_writer.o \
	function.o \
	hasher.o \
	http.o \
	http_impl.o \
	http_java.o \
	json.o \
	json_parse.o \
	module.o \
	scope_exit.o \
	stack_guard.o \
	stdio.o \
	stopwatch.o \
	stopwatch_unix.o \
	thread_reference.o \
	view.o
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
