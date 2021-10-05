%%{
# vim: syntax=ragel:
machine test;

include abnf "abnf.rl";

# field_content
# main := (alpha (" " alpha)?)*;

action A {}
action B {}
action C {}
action D {}
action E {}
action F {}
action G {}
action H {}

# main :=
#   "\r\n"
#   (
#     ([^\r\n]*)
#       >A $B @C %D
#     "\r\n"
#   )*
#   "\r\n";

# main :=
#   ("\r\n" ([^\r\n]+) >A $B @C %D)*
#   "\r\n\r\n";

main :=
  CRLF
  (
    (
      # field_name
      ":"
      OWS
      (
        [a-z]+
        # field_content
          >C $D @E %F
      )*
      OWS
    )*

    CRLF @B
  )*

  CRLF @A

  ;

}%%
