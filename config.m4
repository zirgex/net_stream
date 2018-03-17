dnl
dnl $Id$
dnl config.m4 for extension net_stream

PHP_ARG_ENABLE(net_stream, whether to enable net_stream support,
[  --enable-net_stream           Enable net_stream support])

if test -z "$PHP_ZLIB_DIR"; then
  PHP_ARG_WITH(zlib-dir, for the location of libz,
  [  --with-zlib-dir[=DIR]     net_stream: Set the path to libz install prefix], no, no)
fi

AC_DEFUN([PHP_NET_STREAM_ZLIB],[
  if test "$PHP_ZLIB_DIR" != "no" && test "$PHP_ZLIB_DIR" != "yes"; then
    if test -f "$PHP_ZLIB_DIR/include/zlib/zlib.h"; then
      PHP_ZLIB_DIR="$PHP_ZLIB_DIR"
      PHP_ZLIB_INCDIR="$PHP_ZLIB_DIR/include/zlib"
    elif test -f "$PHP_ZLIB_DIR/include/zlib.h"; then
      PHP_ZLIB_DIR="$PHP_ZLIB_DIR"
      PHP_ZLIB_INCDIR="$PHP_ZLIB_DIR/include"
    else
      AC_MSG_ERROR([Can't find zlib headers under "$PHP_ZLIB_DIR"])
    fi
  else
    for i in /usr/local /usr; do
      if test -f "$i/include/zlib/zlib.h"; then
        PHP_ZLIB_DIR="$i"
        PHP_ZLIB_INCDIR="$i/include/zlib"
      elif test -f "$i/include/zlib.h"; then
        PHP_ZLIB_DIR="$i"
        PHP_ZLIB_INCDIR="$i/include"
      fi
    done
  fi
  if test "$PHP_ZLIB_DIR" != "no" && test "$PHP_ZLIB_DIR" != "yes"; then
    AC_MSG_RESULT([         zlib found in $i])
  else
    AC_MSG_ERROR([         zlib not found])
  fi
])

dnl Various checks for NET_STREAM features
  PHP_NET_STREAM_ZLIB

if test "$PHP_NET_STREAM" != "no" || test "$PHP_NET_STREAM_ENABLED" = "yes"; then
  PHP_SUBST(NET_STREAM_SHARED_LIBADD)
  extra_sources="net_stream.c"
  PHP_NEW_EXTENSION(net_stream, $extra_sources, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
