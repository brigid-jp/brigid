// vim: syntax=ragel:

// Copyright (c) 2021 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

#include "websocket_frame_parser.hpp"

namespace brigid {
  namespace {
    %%{
      machine websocket_frame_parser;

      size16 =
        any @{ payload_length_ = fc << 8; }
        any @{ payload_length_ |= fc; };
      size64 =
        any{8}
          >{ payload_length_ = 0; }
          ${ payload_length_ <<= 8; payload_length_ |= fc; };

      main :=
        (
          any
            @{
              fin_ = fc & 0x80;
              rsv1_ = fc & 0x40;
              rsv2_ = fc & 0x20;
              rsv3_ = fc & 0x10;
              opcode_ = fc & 0x0F;
            }
          (
            0x00..0x7D
              @{
                mask_ = false;
                payload_length_ = fc & 0x7F;
              }
            | 0x7E size16
            | 0x7F size64
          )
          |
          (
            (
              0x80..0xFD
                @{
                  mask_ = true;
                  payload_length_ = fc & 0x7F;
                }
              | 0xFE size16
              | 0xFF size64
            )
            any @{ masking_key_[0] = fc; }
            any @{ masking_key_[1] = fc; }
            any @{ masking_key_[2] = fc; }
            any @{ masking_key_[3] = fc; }
          )
        )
        %{ fbreak; }
        ;

      write data noerror nofinal noentry;
    }%%
  }

  class websocket_frame_parser::impl {
  public:
    impl() {
      reset();
    }

    void reset() {
      %%write init;
      position_ = 0;
      fin_ = false;
      rsv1_ = false;
      rsv2_ = false;
      rsv3_ = false;
      opcode_ = 0;
      mask_ = false;
      payload_length_ = 0;
      masking_key_[0] = '\0';
      masking_key_[1] = '\0';
      masking_key_[2] = '\0';
      masking_key_[3] = '\0';
    }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

    std::pair<parser_state, const char*> parse(const char* data, size_t size) {
      const char* p = data;
      const char* pe = data + size;
      const char* eof = nullptr; // TODO 必要になる条件は？

      %%write exec;

      position_ += p - data;

      if (cs == %%{ write error; }%%) {
        return std::make_pair(parser_state::error, p);
      }
      if (cs >= %%{ write first_final; }%%) {
        return std::make_pair(parser_state::accept, p);
      }
      return std::make_pair(parser_state::running, p);
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    size_t position() const {
      return position_;
    }

    bool fin() const {
      return fin_;
    }

    bool rsv1() const {
      return rsv1_;
    }

    bool rsv2() const {
      return rsv2_;
    }

    bool rsv3() const {
      return rsv3_;
    }

    uint8_t opcode() const {
      return opcode_;
    }

    bool mask() const {
      return mask_;
    }

    uint64_t payload_length() const {
      return payload_length_;
    }

    const char* masking_key() const {
      return masking_key_;
    }

  private:
    int cs;
    size_t position_;
    bool fin_;
    bool rsv1_;
    bool rsv2_;
    bool rsv3_;
    uint8_t opcode_;
    bool mask_;
    uint64_t payload_length_;
    char masking_key_[4];
  };

  websocket_frame_parser::websocket_frame_parser()
    : impl_(new impl()) {}

  websocket_frame_parser::websocket_frame_parser(websocket_frame_parser&& that)
    : impl_(std::move(that.impl_)) {}

  websocket_frame_parser::~websocket_frame_parser() {}

  void websocket_frame_parser::reset() {
    impl_->reset();
  }

  std::pair<parser_state, const char*> websocket_frame_parser::parse(const char* data, size_t size) {
    return impl_->parse(data, size);
  }

  size_t websocket_frame_parser::position() const {
    return impl_->position();
  }

  bool websocket_frame_parser::fin() const {
    return impl_->fin();
  }

  bool websocket_frame_parser::rsv1() const {
    return impl_->rsv1();
  }

  bool websocket_frame_parser::rsv2() const {
    return impl_->rsv2();
  }

  bool websocket_frame_parser::rsv3() const {
    return impl_->rsv3();
  }

  uint8_t websocket_frame_parser::opcode() const {
    return impl_->opcode();
  }

  bool websocket_frame_parser::mask() const {
    return impl_->mask();
  }

  uint64_t websocket_frame_parser::payload_length() const {
    return impl_->payload_length();
  }

  const char* websocket_frame_parser::masking_key() const {
    return impl_->masking_key();
  }
}
