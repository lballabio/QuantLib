
# AM_PATH_QUANTLIB([MINIMUM-VERSION,[ACTION-IF-FOUND,[ACTION-IF-NOT-FOUND]]])
# ---------------------------------------------------------------------------
# Check for QuantLib, and define QUANTLIB_CXXFLAGS and QUANTLIB_LIBS as
# the output of quantlib-config --cflags and quantlib-config --libs,
# respectively
AC_DEFUN([AM_PATH_QUANTLIB],
[AC_ARG_ENABLE(quantlib-test,
               AC_HELP_STRING([--disable-quantlib-test],
                              [do not try to compile and run a test QuantLib program]),
		       [enable_quantlib_test=$enableval],
		       [enable_quantlib_test=yes])
 AC_ARG_WITH(quantlib-prefix,
             AC_HELP_STRING([--with-quantlib-prefix=PREFIX],
                            [prefix where QuantLib is installed]),
             quantlib_prefix="$withval", quantlib_prefix="")
 AC_ARG_WITH(quantlib-exec-prefix,
             AC_HELP_STRING([--with-quantlib-exec-prefix=PREFIX],
                            [exec prefix where QuantLib is installed]),
             quantlib_exec_prefix="$withval", quantlib_exec_prefix="")

  no_quantlib=no

  # First, look for quantlib-config.

  # If a prefix or exec-config was given, look there.
  if test x$quantlib_exec_prefix != x ; then
    if test x${QUANTLIB_CONFIG+set} != xset ; then
      QUANTLIB_CONFIG=$quantlib_exec_prefix/bin/quantlib-config
    fi
  fi
  if test x$quantlib_prefix != x ; then
    if test x${QUANTLIB_CONFIG+set} != xset ; then
      QUANTLIB_CONFIG=$quantlib_prefix/bin/quantlib-config
    fi
  fi

  AC_PATH_PROG(QUANTLIB_CONFIG, quantlib-config, no)

  if test "$QUANTLIB_CONFIG" = "no" ; then
    no_quantlib=yes
  else
    QUANTLIB_CXXFLAGS=`$QUANTLIB_CONFIG --cflags`
    QUANTLIB_LIBS=`$QUANTLIB_CONFIG --libs`
    QUANTLIB_VERSION=`$QUANTLIB_CONFIG --version`
  fi

  
  # Supposing we found quantlib-config...
  if test $no_quantlib != yes ; then

    # ...we check for the required version (if any)
    if test x$1 != x ; then
      AC_MSG_CHECKING([for QuantLib version ($1 or later required)])

      major=`echo $QUANTLIB_VERSION | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\(\.\([[0-9]]*\)\)*\([[a|b|c]][[0-9]]*\)*/\1/'`
      wanted=`echo $1 | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\(\.\([[0-9]]*\)\)*\([[a|b|c]][[0-9]]*\)*/\1/'`
      if test $major -lt $wanted ; then
        AC_MSG_RESULT(no)
        no_quantlib=yes
      else
        if test $major -gt $wanted ; then
          AC_MSG_RESULT($QUANTLIB_VERSION)
        else
          minor=`echo $QUANTLIB_VERSION | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\(\.\([[0-9]]*\)\)*\([[a|b|c]][[0-9]]*\)*/\2/'`
          wanted=`echo $1 | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\(\.\([[0-9]]*\)\)*\([[a|b|c]][[0-9]]*\)*/\2/'`
          if test $minor -lt $wanted ; then
            AC_MSG_RESULT(no)
            no_quantlib=yes
          else
            if test $minor -gt $wanted ; then
              AC_MSG_RESULT($QUANTLIB_VERSION)
            else
              patch=`echo $QUANTLIB_VERSION | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\(\.\([[0-9]]*\)\)*\([[a|b|c]][[0-9]]*\)*/\4/'`
              if test x$patch = x ; then
                patch=0
              fi
              wanted=`echo $1 | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\(\.\([[0-9]]*\)\)*\([[a|b|c]][[0-9]]*\)*/\4/'`
              if test x$wanted = x ; then
                wanted=0
              fi
              if test $patch -lt $wanted ; then
                AC_MSG_RESULT(no)
                no_quantlib=yes
              else
                AC_MSG_RESULT($QUANTLIB_VERSION)
              fi
            fi
          fi
        fi
      fi
    else
      AC_MSG_CHECKING([for QuantLib version])
      AC_MSG_RESULT($QUANTLIB_VERSION)
    fi

  fi

  # If we have an up-to-date library...
  if test $no_quantlib != yes ; then

    # ...we test it (unless we were told not to.)
    if test "x$enable_quantlib_test" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $QUANTLIB_CXXFLAGS"
      LIBS="$LIBS $QUANTLIB_LIBS"

      AC_MSG_CHECKING([that we can compile and link QuantLib programs])

      AC_TRY_RUN([
@%:@include <ql/quantlib.hpp>

int main (int argc, char *argv[]) {
    return 0;
}
      ],
        [AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)
         no_quantlib=yes])

      CXXFLAGS="$ac_save_CXXFLAGS"
      LIBS="$ac_save_LIBS"
    fi

  fi

  # All tests were performed.  Based on the result, we trigger the
  # appropriate action (if any)
    
  if test "x$no_quantlib" = xno ; then
     ifelse([$2], , :, [$2])
  else
     QUANTLIB_CXXFLAGS=""
     QUANTLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_SUBST(QUANTLIB_CXXFLAGS)
  AC_SUBST(QUANTLIB_LIBS)

])
