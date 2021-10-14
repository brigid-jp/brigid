%%{
# vim: syntax=ragel:
machine string01;

action A {}
action B {}
action C {}
action D {}
action E {}
action F {}
action G {}

string_impl2 :=
  ( "\"" @E
  | [A-Z]+ "\"" @F
  | "\\" [a-z] @G
  );

string_impl1 :=
  ( "\"" @B
  | [A-Z]+ "\"" @C
  | "\\" [a-z] @D
  );

main := "\"" @A;

}%%
