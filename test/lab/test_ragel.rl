// vim: syntax=cpp:
#include <iostream>

%%{
  machine websocket;
  request_line = 'HTTP/' digit+ '/' digit+ '\r\n';
  main := request_line;
}%%

%%write data;

void test_ragel(int cs, const char* p, const char* pe) {
  %%write init;
  %%write exec;
}
