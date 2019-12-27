#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

generate() {
  case X$2 in
    X) ;;
    *) printf '#define %s 0x%08X\n' "$1" "$2";;
  esac
}

generate BRIGID_ZONE1 "$BRIGID_ZONE1"




