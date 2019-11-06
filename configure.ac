m4_include([version.m4])
AC_INIT([brigid-core], DROMOZOA_UNIX_VERSION, [dev@brigid.jp], [], [https://github.com/brigid-jp/brigid-core/])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11)

AX_PROG_LUA([5.1], [], [], [AC_MSG_ERROR([could not find lua])])
AX_LUA_HEADERS([], [AC_MSG_ERROR([could not find lua])])

AX_CHECK_OPENSSL([
  AC_DEFINE(HAVE_OPENSSL, 1, [Define to 1 if using OpenSSL.])
  LDFLAGS="$LDFLAGS $OPENSSL_LDFLAGS"
  LIBS="$OPENSSL_LIBS $LIBS"
  CPPFLAGS="$OPENSSL_INCLUDES $CPPFLAGS"
  have_openssl=yes
])

AC_CHECK_HEADERS([CommonCrypto/CommonCrypto.h], [
  have_commoncrypto=yes
])

AM_CONDITIONAL([HAVE_OPENSSL], [test "X$have_openssl" = Xyes])
AM_CONDITIONAL([HAVE_COMMONCRYPTO], [test "X$have_commoncrypto" = Xyes])

AC_OUTPUT
