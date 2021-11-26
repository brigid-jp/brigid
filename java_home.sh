#! /bin/sh -e

# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

if test "X$JAVA_HOME" = X
then
  if test -x /usr/libexec/java_home
  then
    /usr/libexec/java_home
  else
    path=:$PATH
    while :
    do
      i=`expr "X$path" : 'X\(:[^:]*\)' | sed 's/^.//'`
      case X$i in
        X) i=.;;
      esac
      if test -x "$i/javac"
      then
        i=`(cd "$i" && pwd)`
        i=`readlink -f "$i/javac"`
        i=`expr "X:$i" : 'X\(:.*\)/bin/javac' '|' "X:$i" : 'X\(:.*\)/javac' '|' X`
        case X$i in
          XX) ;;
          *) echo "$i" | sed 's/^.//'; break;;
        esac
      fi
      path=`expr "X$path" : 'X:[^:]*\(:.*\)' '|' X`
      case X$path in
        XX) break;;
      esac
    done
  fi
else
  echo "$JAVA_HOME"
fi
