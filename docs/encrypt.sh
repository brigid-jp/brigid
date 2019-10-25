#! /bin/sh -e

openssl enc -e -aes-256-cbc -in plain.txt -out encrypted.dat -pass pass:test -p
