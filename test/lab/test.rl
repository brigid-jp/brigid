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
#  CRLF
#  (
#    (
#      # field_name
#      [a-z]+
#      ":"
#      OWS
#      (
#        # field_content
#        [a-z]+
#          >C $D @E %F
#        |
#        # fold
#        CRLF " "
#          @H
#      )*
#      OWS
#    )*
#
#    CRLF @B
#  )*
#
#  CRLF @A

        "GET"

        CRLF

        (
          (
            field_name
              $A # { field_name_ += fc; }
              # %{ std::cout << "field_name [" << field_name_ << "]\n"; }

            ":"
            OWS
            (
              field_content
                $B # { field_value_ += fc; }
              |
              obs_fold
                @C
                # {
                #   std::cout << "obs_fold\n";
                #   field_value_ += ' ';
                # }
            )*
            OWS
          )

          CRLF
            %D
            # {
            #   // std::cout << "field_value [" << field_value_ << "]\n";
            #   std::cout << "CRLF\n";
            #   header_fields_.insert(std::make_pair(field_name_, field_value_));
            #   field_name_.clear();
            #   field_value_.clear();
            # }
        )*

        CRLF @E # { fbreak; }
  ;

}%%
