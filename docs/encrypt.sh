#! /bin/sh -e

openssl enc -e -aes-256-cbc -in plain.txt -out encrypted.dat -p
