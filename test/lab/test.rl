%%{
# vim: syntax=ragel:
machine test;

# include abnf "abnf.rl";

action A {}
action B {}
action C {}
action D {}
action E {}
action F {}
action G {}
action H {}
action X {}
action Y {}
action Z {}

# unescaped = [^\\\"];
#
# escaped
#   = "\\\\" @X
#   | "\\\"" @Y
#   ;
#
# main := "\"" %A
#     "\"" @B1
#   | unescaped+
#     ( "\"" @B2
#     | escaped >C1 (escaped | unescaped @D)* "\"" @B3
#     )
#   | escaped >C2 (escaped | unescaped @D)* "\"" @B4
# ;

# 0000..D7FF
#   0-C .   ..
#   D   0-7 ..
#
# E000..FFFF
#   E ...
#   F ...

# D800..DBFF DC00..DFFF
#   D 8-B ..
#   D C-F ..

# Char. number range  |        UTF-8 octet sequence
#    (hexadecimal)    |              (binary)
# --------------------+---------------------------------------------
# 0000 0000-0000 007F | 0xxxxxxx
# 0000 0080-0000 07FF | 110xxxxx 10xxxxxx
# 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
# 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

# 0x0000 0x007F | 0xxxxxxx
# 0x0080 0x07FF | 110xxxxx 10xxxxxx
# 0x0800 0xFFFF | 1110xxxx 10xxxxxx 10xxxxxx


xdig
  = [0-9] @X
  | [A-F] @Y
  | [a-f] @Z
  ;

# 0xD800 0xDBFF
high_surrogate = /D[89AB][0-9A-F][0-9A-F]/i ;

# 0xDC00 0xDFFF
low_surrogate = /D[C-F][0-9A-F][0-9A-F]/i;

surrogate = high_surrogate | low_surrogate;

utf16_utf8_1 = /00[0-7][0-9A-F]/i;

utf16_utf8_2 = /00[89A-F][0-9A-F]/i | /0[1-7][0-9A-F][0-9A-F]/i;

utf16_utf8_3 = (/0[89A-F][0-9A-F][0-9A-F]/i | /[1-9A-F][0-9A-F][0-9A-F][0-9A-F]/i) - surrogate;

main :=
  "\\u"
  ( (xdig{4} & utf16_utf8_1) %A
  | (xdig{4} & utf16_utf8_2) %B
  | (xdig{4} & utf16_utf8_3) %C
  | (xdig{4} & high_surrogate) %D "\\u" (xdig{4} & low_surrogate) %E
  )

  ;

}%%
