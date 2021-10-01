// vim: syntax=cpp:

#include "test_ragel.hpp"

namespace brigid {
  namespace {
    %%{
      machine websocket;
      request_line = 'HTTP/' digit+ '/' digit+ '\r\n';
      main := request_line;
    }%%

    %%write data;
  }

  class websocket_request_parser::impl {
  public:
    impl() {
      %%write init;
    }

    void update(const char* p, const char* pe) {
      %%write exec;
    }

  private:
    int cs;
  };

  websocket_request_parser::websocket_request_parser()
    : impl_(new impl()) {}

  websocket_request_parser::~websocket_request_parser() {}

  void websocket_request_parser::update(const char* data, size_t size) {
    impl_->update(data, data + size);
  }
}
