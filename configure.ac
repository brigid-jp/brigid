# Copyright (c) 2019-2021,2024 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

m4_include([brigid_version.m4])
AC_INIT([brigid], BRIGID_VERSION, [dev@brigid.jp], [], [https://github.com/brigid-jp/brigid/])
AM_INIT_AUTOMAKE([foreign])
LT_INIT
gl_LD_VERSION_SCRIPT

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/lua/Makefile test/lua/Makefile])

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11, noext)

case X$OBJCXXFLAGS in
  X) OBJCXXFLAGS="-fobjc-arc";;
  *) OBJCXXFLAGS="$OBJCXXFLAGS -fobjc-arc";;
esac
case X$HAVE_CXX11 in
  X1)
    OBJCXXFLAGS="$OBJCXXFLAGS -std=c++11";;
esac
AC_PROG_OBJCXX

AX_PROG_LUA([5.1], [], [], [AC_MSG_ERROR([could not find lua])])
AX_LUA_HEADERS([], [AC_MSG_ERROR([could not find lua])])

AC_CHECK_HEADERS([CommonCrypto/CommonCrypto.h], [
  AC_DEFINE(CRYPTO_APPLE, 1, [Define to 1 if using CommonCrypto.])
  crypto_apple=yes
])
AM_CONDITIONAL([CRYPTO_APPLE], [test "X$crypto_apple" = Xyes])

AX_CHECK_OPENSSL([
  AC_DEFINE(CRYPTO_OPENSSL, 1, [Define to 1 if using OpenSSL.])
  crypto_openssl=yes
])
AM_CONDITIONAL([CRYPTO_OPENSSL], [test "X$crypto_openssl" = Xyes])

AC_LANG_PUSH([Objective C++])
AC_CHECK_HEADERS([Foundation/Foundation.h], [
  AC_DEFINE(HTTP_APPLE, 1, [Define to 1 if using NSURLSession.])
  http_apple=yes
])
AC_LANG_POP([Objective C++])
AM_CONDITIONAL([HTTP_APPLE], [test "X$http_apple" = Xyes])

AX_LIB_CURL([7.17.0], [
  AC_DEFINE(HTTP_CURL, 1, [Define to 1 if using libcurl.])
  http_curl=yes
])
AM_CONDITIONAL([HTTP_CURL], [test "X$http_curl" = Xyes])

AC_CHECK_FUNCS([dladdr dlopen])

AC_OUTPUT
