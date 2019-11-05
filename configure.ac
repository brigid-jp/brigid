m4_include([version.m4])
AC_INIT([brigid-core], DROMOZOA_UNIX_VERSION, [dev@brigid.jp], [], [https://github.com/brigid-jp/brigid-core/])
AM_INIT_AUTOMAKE
LT_INIT

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

AC_PROG_CXX

AX_PROG_LUA([5.1], [], [], [AC_MSG_ERROR([could not find lua])])
AX_LUA_HEADERS([], [AC_MSG_ERROR([could not find lua])])

AC_OUTPUT
