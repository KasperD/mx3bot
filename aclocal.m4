dnl Sets VERSION and BASE_VERSION.
AC_DEFUN(MX3BOT, [
  if test "" = "$2"; then
    VERSION="$1"
  else
    VERSION="$1.$2"
  fi

  BASE_VERSION="$1"

  AC_SUBST(VERSION)
  AC_SUBST(BASE_VERSION)
  AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
])


dnl Check --with/--enable options.
AC_DEFUN(MX3BOT_OPT_MYSQL, [
  AC_ARG_WITH(mysql,
    [  --with-mysql			compile in MySQL support],
    [want_mysql=$withval],
    [want_mysql=no])
])

AC_DEFUN(MX3BOT_OPT_PHPNUKE, [
  AC_ARG_ENABLE(phpnuke,
    [  --enable-phpnuke		compile in PHPNuke functions],
    [want_mysql=yes
     AC_DEFINE(ENABLE_PHPNUKE)],
    [if test $want_mysql = yes; then
       AC_DEFINE(ENABLE_PHPNUKE)
     fi])
])

AC_DEFUN(MX3BOT_OPT_FXSITE, [
  AC_ARG_ENABLE(fxsite,
    [  --enable-fxsite		compile in fxSite functions],
    [want_mysql=yes
     AC_DEFINE(ENABLE_FXSITE)],
    [if test $want_mysql = yes; then
       AC_DEFINE(ENABLE_FXSITE)
     fi])
])

AC_DEFUN(MX3BOT_OPT_TCL_LIB, [
  AC_ARG_WITH(tcl-lib,
    [  --with-tcl-lib=PATH		specify path to Tcl library],
    [tcl_lib=$withval],
    [tcl_lib=no])
])

AC_DEFUN(MX3BOT_OPT_TCL_INCLUDE, [
  AC_ARG_WITH(tcl-include,
    [  --with-tcl-include=PATH	specify path to Tcl include files],
    [tcl_inc=$withval],
    [tcl_inc=no])
])

AC_DEFUN(MX3BOT_OPT_MYSQL_LIB, [
  AC_ARG_WITH(mysql-lib,
    [  --with-mysql-lib=PATH		specify path to MySQL client library],
    [mysql_lib=$withval],
    [mysql_lib=no])
])

AC_DEFUN(MX3BOT_OPT_MYSQL_INCLUDE, [
  AC_ARG_WITH(mysql-include,
    [  --with-mysql-include=PATH	specify path to MySQL include files],
    [mysql_inc=$withval],
    [mysql_inc=no])
])

AC_DEFUN(MX3BOT_OPT_REGEX, [
  AC_ARG_WITH(regex,
    [  --with-regex			use regular expression mask matching],
    [want_regex=$withval],
    [want_regex=default])
])


dnl Check for Tcl libraries and includes.
AC_DEFUN(MX3BOT_CHECK_TCL, [
  tcl_names="tcl tcl8.4 tcl8.3 tcl8.2 tcl8.1 tcl8.0 tcl8 tcl84 tcl83 \
    tcl82 tcl81 tcl80"

  MX3BOT_CHECK_TCL_LIB
  MX3BOT_CHECK_TCL_INCLUDE
])


dnl Look in various places for Tcl library
AC_DEFUN(MX3BOT_CHECK_TCL_LIB, [
  search="/usr/local/lib /usr/lib /lib /usr/local/tcl/lib /usr/lib/tcl"

  # first.. does it work already?
  AC_SEARCH_LIBS(Tcl_Eval, [$tcl_names], [found=yes], [found=no])

  if test $found = no; then
    # add user supplied path if given
    if ! test $tcl_lib = no; then
      search="$tcl_lib $search"
    fi

    AC_MSG_CHECKING(for a directory with Tcl library)
    AC_MSG_RESULT()

    for path in $search; do
      orig=$LDFLAGS
      LDFLAGS="-L$path $LDFLAGS"

      unset ac_cv_search_Tcl_Eval
      AC_SEARCH_LIBS(Tcl_Eval, [$tcl_names], [found=yes])

      if test $found = yes; then
        break
      fi

      LDFLAGS=$orig
    done
  fi

  if test $found = no; then
    AC_MSG_ERROR(Could not locate Tcl library.  Try --with-tcl-library.)
  fi
])


dnl Look in various places for tcl.h
AC_DEFUN(MX3BOT_CHECK_TCL_INCLUDE, [
  search="/usr/local/include /usr/include"

  # check in default include paths
  AC_CHECK_HEADER(tcl.h, [found=yes], [found=no])

  if test $found = no; then
    # add user supplied include path if given
    if ! test $tcl_inc = no; then
      search="$tcl_inc $search"
    fi

    AC_MSG_CHECKING(for a directory with tcl.h)
    AC_MSG_RESULT()

    # look for tcl.h in base directories
    for path in $search; do
      if test -r "$path/tcl.h"; then
        orig=$CPPFLAGS
        CPPFLAGS="-I$path $CPPFLAGS"

        AC_MSG_CHECKING($path/tcl.h)
        AC_TRY_COMPILE([#include <tcl.h>], [], [found=yes])
        AC_MSG_RESULT($found)

        if test $found = yes; then
          break
        fi

        CPPFLAGS=$orig
      fi
    done
  fi

  if test $found = no; then
    for base in $search; do
      for path in $tcl_names; do
        if test -r "$base/$path/tcl.h"; then
          orig=$CPPFLAGS
          CPPFLAGS="-I$base/$path $CPPFLAGS"

          AC_MSG_CHECKING($base/$path/tcl.h)
          AC_TRY_COMPILE([#include <tcl.h>], [], [found=yes])
          AC_MSG_RESULT($found)

          if test $found = yes; then
            break 2
          fi

          CPPFLAGS=$orig
        fi
      done
    done
  fi

  if test $found = no; then
    AC_MSG_ERROR(Could not locate tcl.h.  Try --with-tcl-include)
  fi
])


dnl Check for MySQL if wanted.
AC_DEFUN(MX3BOT_CHECK_MYSQL, [
  if test "$want_mysql" = yes; then
    MX3BOT_CHECK_MYSQL_LIB
    MX3BOT_CHECK_MYSQL_INCLUDE

    AC_DEFINE(HAVE_SQL)
    AC_DEFINE(HAVE_MYSQL)
  fi
])


dnl Look in various places for libmysqlclient.
AC_DEFUN(MX3BOT_CHECK_MYSQL_LIB, [
  search="/usr/local/lib /usr/local/lib/mysql /usr/local/mysql		\
    /usr/local/mysql/lib /usr/local/mysql/lib/mysql /lib /lib/mysql"

  # check default settings
  AC_CHECK_LIB(mysqlclient, mysql_real_connect, [found=yes], [found=no])

  if test $found = no; then
    if test "x$mysql_lib" != "xno"; then
      search="$mysql_lib $search"
    fi

    for path in $search; do
      orig=$LDFLAGS
      LDFLAGS="-L$path $LDFLAGS"

      unset ac_cv_lib_mysqlclient_mysql_real_connect
      AC_CHECK_LIB(mysqlclient, mysql_real_connect, [found=yes])

      if test $found = yes; then
        break
      fi

      LDFLAGS=$orig
    done
  fi

  if test $found = no; then
    AC_MSG_ERROR(Could not locate MySQL client library.  Try --with-mysql-lib)
  fi

  LIBS="$LIBS -lmysqlclient"
])


dnl Look in various places for mysql.h
AC_DEFUN(MX3BOT_CHECK_MYSQL_INCLUDE, [
  search="/usr/local/include /usr/local/include/mysql 			\
    /usr/local/mysql/include/mysql /usr/include/mysql"

  # check in default include paths
  AC_CHECK_HEADER(mysql.h, [found=yes], [found=no])

  if test $found = no; then
    # add user supplied include path if given
    if test "x$mysql_inc" = xno; then
      search="$mysql_inc $search"
    fi

    AC_MSG_CHECKING(for a directory with mysql.h)
    AC_MSG_RESULT()

    for path in $search; do
      if test -r "$path/mysql.h"; then
        orig=$CPPFLAGS
        CPPFLAGS="-I$path $CPPFLAGS"

        AC_MSG_CHECKING($path/mysql.h)
        AC_TRY_COMPILE([#include <mysql.h>], [], [found=yes])
        AC_MSG_RESULT($found)

        if test $found = yes; then
          break
        fi

        CPPFLAGS=$orig
      fi
    done
  fi

  if test $found = no; then
    AC_MSG_ERROR(Could not locate mysql.h.  Try --with-mysql-include)
  fi
])


dnl Check that regcomp/regexec actually work.
AC_DEFUN(MX3BOT_CHECK_REGEX, [

  AC_MSG_CHECKING(for working regcomp/regexec/regfree)

  AC_TRY_RUN([
    #include <sys/types.h>
    #include <regex.h>
    int main(void) {
       regex_t reg;
       if(regcomp(&reg, ".*!~?blah@ppp-[0-9]+\\.isp\\.com",
          REG_ICASE | REG_NOSUB | REG_EXTENDED) != 0) exit(1);
       if(regexec(&reg, "nick!~blah@PPP-206.isp.com", 0, 0, 0) != 0)
          exit(1);
       exit(0); }],
  [regex=yes], [regex=no], [regex=cross])

  if test $regex = cross; then
    if test $want_regex = yes; then
      AC_DEFINE(HAVE_REGEX)
      AC_MSG_RESULT(assumed working)
    else
      AC_MSG_RESULT(not tested)
    fi
  fi

  if test $regex = yes; then
    AC_DEFINE(HAVE_REGEX)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)

    if test $want_regex = yes; then
      AC_MSG_ERROR(POSIX regex functions do not work.)
    fi
  fi
])
