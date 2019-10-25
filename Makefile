CPPFLAGS += -I$(LUA_INCDIR)
CXXFLAGS += -Wall -W $(CFLAGS)
LDFLAGS += -L$(LUA_LIBDIR) $(LIBFLAG)
LDLIBS += -lssl -lcrypto -ldl

OBJS = \
	module.o
TARGET = brigid_core.so

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

check:
	./test.sh

brigid_core.so: $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

install:
	cp $(TARGET) $(LIBDIR)
