for i in test/lua/test*.lua
do
  case X$# in
    X0) lua "$i";;
    *) "$@" "$i";;
  esac
done
