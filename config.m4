dnl $Id$
dnl config.m4 for extension dyjcrypt

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(dyjcrypt, for dyjcrypt support,
Make sure that the comment is aligned:
[  --with-dyjcrypt             Include dyjcrypt support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(dyjcrypt, whether to enable dyjcrypt support,
Make sure that the comment is aligned:
[  --enable-dyjcrypt           Enable dyjcrypt support])

if test "$PHP_DYJCRYPT" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-dyjcrypt -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/dyjcrypt.h"  # you most likely want to change this
  dnl if test -r $PHP_DYJCRYPT/$SEARCH_FOR; then # path given as parameter
  dnl   DYJCRYPT_DIR=$PHP_DYJCRYPT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for dyjcrypt files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       DYJCRYPT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$DYJCRYPT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the dyjcrypt distribution])
  dnl fi

  dnl # --with-dyjcrypt -> add include path
  dnl PHP_ADD_INCLUDE($DYJCRYPT_DIR/include)

  dnl # --with-dyjcrypt -> check for lib and symbol presence
  dnl LIBNAME=dyjcrypt # you may want to change this
  dnl LIBSYMBOL=dyjcrypt # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DYJCRYPT_DIR/$PHP_LIBDIR, DYJCRYPT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_DYJCRYPTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong dyjcrypt lib version or lib not found])
  dnl ],[
  dnl   -L$DYJCRYPT_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(DYJCRYPT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(dyjcrypt, dyjcrypt.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
