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

main :=
  ( "-"? ("0" | [1-9] digit*)
    ( ("." @A digit+) ([eE] [+\-]? digit+)?
    | ([eE] @B [+\-]? digit+)?
    )
  );

}%%
