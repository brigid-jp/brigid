// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#ifndef BRIGID_WEBSOCKET_FRAME_PARSER_HPP
#define BRIGID_WEBSOCKET_FRAME_PARSER_HPP

// parser_state
#include "http_request_parser.hpp"

#include <stddef.h>
#include <memory>
#include <utility>

namespace brigid {
  class websocket_frame_parser {
  public:
    websocket_frame_parser();
    websocket_frame_parser(websocket_frame_parser&&);
    ~websocket_frame_parser();
    void reset();
    std::pair<parser_state, const char*> parse(const char*, size_t);
    size_t position() const;
    bool fin() const;
    bool rsv1() const;
    bool rsv2() const;
    bool rsv3() const;
    uint8_t opcode() const;
    bool mask() const;
    uint64_t payload_length() const;
    const char* masking_key() const;
  private:
    class impl;
    std::unique_ptr<impl> impl_;
  };
}

#endif
