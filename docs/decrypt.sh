#! /bin/sh -e

openssl enc -d -aes-256-cbc -in encrypted.dat -out decrypted.txt -pass pass:test -p
