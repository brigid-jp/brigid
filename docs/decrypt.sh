#! /bin/sh -e

# Copyright (c) 2019 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

# https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
plain="The quick brown fox jumps over the lazy dog"
# "0123456789012345"
key128=30313233343536373839303132333435
# "012345678901234567890123"
key192=303132333435363738393031323334353637383930313233
# "01234567890123456789012345678901"
key256=3031323334353637383930313233343536373839303132333435363738393031
# "0123456778012345"
iv=30313233343536373839303132333435

result=`openssl enc -d -aes-128-cbc -in aes-128-cbc.dat -K "$key128" -iv "$iv"`
if test "X$plain" = "X$result"
then
  echo "PASS aes-128-cbc"
else
  echo "FAIL aes-128-cbc"
fi

result=`openssl enc -d -aes-192-cbc -in aes-192-cbc.dat -K "$key192" -iv "$iv"`
if test "X$plain" = "X$result"
then
  echo "PASS aes-192-cbc"
else
  echo "FAIL aes-192-cbc"
fi

result=`openssl enc -d -aes-256-cbc -in aes-256-cbc.dat -K "$key256" -iv "$iv"`
if test "X$plain" = "X$result"
then
  echo "PASS aes-256-cbc"
else
  echo "FAIL aes-256-cbc"
fi
