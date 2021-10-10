%%{
# vim: syntax=ragel:
machine unicode01;

action digit {}
action upper {}
action lower {}

action init1 {}
action init2 {}
action utf8_1 {}
action utf8_2 {}
action utf8_3 {}
action utf8_4 {}

hex_quad =
  ( [0-9] @digit
  | [A-F] @upper
  | [a-f] @lower
  ){4};

# U+0000..U+007F
hex16_1 = /00[0-7][0-9A-F]/i;

# U+0080..U+07FF
hex16_2 =
  ( /00[89A-F][0-9A-F]/i
  | /0[1-7][0-9A-F][0-9A-F]/i
  );

# U+0800..U+FFFF - U+D800..U+DFFF = U+0800..U+D7FF, U+E000..U+FFFF
hex16_3 =
  ( /0[89A-F][0-9A-F][0-9A-F]/i
  | /[1-9A-C][0-9A-F][0-9A-F][0-9A-F]/i
  | /D[0-7][0-9A-F][0-9A-F]/i
  | /[EF][0-9A-F][0-9A-F][0-9A-F]/i
  );

# U+D800..U+DBFF
hex16_h = /D[89AB][0-9A-F][0-9A-F]/i;

# U+DC00..U+DFFF
hex16_l = /D[C-F][0-9A-F][0-9A-F]/i;

unicode_escape_sequence =
  "\\u" @init1
  ( (hex_quad & hex16_1) %utf8_1
  | (hex_quad & hex16_2) %utf8_2
  | (hex_quad & hex16_3) %utf8_3
  | (hex_quad & hex16_h) "\\u" @init2 (hex_quad & hex16_l) %utf8_4
  );

main := unicode_escape_sequence*;

}%%
