%%{
# vim: syntax=ragel:
machine test;

# include abnf "abnf.rl";

action A {}
action B1 {}
action B2 {}
action B3 {}
action B4 {}
action C1 {}
action C2 {}
action D {}
action E {}
action F {}
action G {}
action H {}
action X {}
action Y {}

unescaped = [^\\\"];

escaped
  = "\\\\" @X
  | "\\\"" @Y
  ;

main := "\"" %A
    "\"" @B1
  | unescaped+
    ( "\"" @B2
    | escaped >C1 (escaped | unescaped @D)* "\"" @B3
    )
  | escaped >C2 (escaped | unescaped @D)* "\"" @B4
;

}%%
