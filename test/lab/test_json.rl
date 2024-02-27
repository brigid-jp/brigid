// vim: syntax=ragel:
#include <string.h>
#include <iostream>

%%{
# vim: syntax=ragel:
machine test;

action A { std::cout << "[A] ts-te: " << std::string(ts, te) << "\n"; }
action B {}
action C {}
action D {}
action E {}
action F {}
action G {}
action H {}
action X { std::cout << "[X] ts-te: " << std::string(ts, te) << "\n"; }
action Y { std::cout << "[Y] ts-te: " << std::string(ts, te) << "\n"; }
action Z { std::cout << "[Z] ts-te: " << std::string(ts, te) << "\n"; }

esc
  = [ab] @X
  | "g01" @Y
  | "g02" @Z
  ;

un1 = [cd];
un2 = [ef] [0-2];
un3 = [gh] [0-2] [0-2];

# NG: cccg01
# main :=
#   (
#     ( esc
#     | (un1 @A | un2 @B | un3 @C) (un1|un2|un3)* %F
#     )**
#   )+
#   ;

# main := |*
#   (un1|un2|un3)+ => A;
#   [ab] => X;
#   "g01" => Y;
#   "g02" => Z;
# *|;

# main := 
#   ( (un1@A|un2@B|un3@C) (un1|un2|un3)* %F (esc*)
#   | (esc+)
#   );

  main :=
    ( ([a-b] @{
        // if (ps) {
        //   std::cout << "?un: " << std::string(ps, fpc) << "\n";
        //   ps = nullptr;
        // }
        std::cout << "esc: " << fc << "\n";
      })+
    | (un1|un2|un3)>{ps = fpc;} (un1|un2|un3)* %{
      if (ps) {
        std::cout << "!un: " << std::string(ps, fpc) << "\n";
        // ps = nullptr;
      }
      }
    )**
    ;

}%%

int main(int ac, char* av[]) {
  %%write data noerror nofinal noentry;

  int cs = 0;
  int act = 0;
  const char* ts = nullptr;
  const char* te = nullptr;

  %%write init;

  const char* const pb = av[1];
  const char* p = pb;
  const char* const pe = p + strlen(pb);
  const char* const eof = pe;

  const char* ps = nullptr;

  %%write exec;

  std::cout
    << "cs: " << cs << "\n"
    << "ff: " << %%{ write first_final; }%% << "\n";
}
