CPPFLAGS += -I$(LUA_INCDIR)
CXXFLAGS += -std=c++11 -Wall -W $(CFLAGS)
LDLIBS += -lssl -lcrypto -ldl

OBJS = \
	evp.o \
	module.o
TARGET = brigid_core.so

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

check:
	./test.sh

brigid_core.so: $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBFLAG) $^ $(LDLIBS) -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

install:
	cp $(TARGET) $(LIBDIR)
