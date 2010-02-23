
# Configure paths for QUANTLIB
# copied from the SDL library

# AM_PATH_QUANTLIB([MINIMUM-VERSION,[ACTION-IF-FOUND,[ACTION-IF-NOT-FOUND]]])
# ---------------------------------------------------------------------------
# Check for QuantLib, and define QUANTLIB_CXXFLAGS and QUANTLIB_LIBS as 
# the output of quantlib-config --cflags and quantlib-config --libs,
# respectively
AC_DEFUN([AM_PATH_QUANTLIB],
[AC_ARG_ENABLE(quantlib-test,
               AC_HELP_STRING([--disable-quantlib-test],
                              [do not try to compile and run a test QuantLib program]),
		       enable_quantlib_test=yes)
 AC_ARG_WITH(quantlib-prefix,
             AC_HELP_STRING([--with-quantlib-prefix=PREFIX],
                            [prefix where QuantLib is installed]),
             quantlib_prefix="$withval", quantlib_prefix="")
 AC_ARG_WITH(quantlib-exec-prefix,
             AC_HELP_STRING([--with-quantlib-exec-prefix=PREFIX],
                            [exec prefix where QuantLib is installed]),
             quantlib_exec_prefix="$withval", quantlib_exec_prefix="")

  if test x$quantlib_exec_prefix != x ; then
     quantlib_args="$quantlib_args --exec-prefix=$quantlib_exec_prefix"
     if test x${QUANTLIB_CONFIG+set} != xset ; then
        QUANTLIB_CONFIG=$quantlib_exec_prefix/bin/quantlib-config
     fi
  fi
  if test x$quantlib_prefix != x ; then
     quantlib_args="$quantlib_args --prefix=$quantlib_prefix"
     if test x${QUANTLIB_CONFIG+set} != xset ; then
        QUANTLIB_CONFIG=$quantlib_prefix/bin/quantlib-config
     fi
  fi

  AC_PATH_PROG(QUANTLIB_CONFIG, quantlib-config, no)
  min_quantlib_version=ifelse([$1], ,0.2.0,$1)
  AC_MSG_CHECKING([QuantLib version ($min_quantlib_version or later required)])
  no_quantlib=""
  if test "$QUANTLIB_CONFIG" = "no" ; then
    no_quantlib=yes
  else
    QUANTLIB_CXXFLAGS=`$QUANTLIB_CONFIG $quantlibconf_args --cflags`
    QUANTLIB_LIBS=`$QUANTLIB_CONFIG $quantlibconf_args --libs`

    quantlib_major_version=`$QUANTLIB_CONFIG $quantlib_args --version | \
           sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)\([[a|b|c]][[0-9]]*\)*\(-cvs\)*/\1/'`
    quantlib_minor_version=`$QUANTLIB_CONFIG $quantlib_args --version | \
           sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)\([[a|b|c]][[0-9]]*\)*\(-cvs\)*/\2/'`
    quantlib_micro_version=`$QUANTLIB_CONFIG $quantlib_config_args --version | \
           sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)\([[a|b|c]][[0-9]]*\)*\(-cvs\)*/\3/'`
    if test "x$enable_quantlibtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $QUANTLIB_CXXFLAGS"
      LIBS="$LIBS $QUANTLIB_LIBS"

# Now check if the installed QuantLib is sufficiently new. (Also sanity
# checks the results of quantlib-config to some extent

      rm -f conf.quantlibtest
      AC_TRY_RUN([
@%:@include <stdio.h>
@%:@include <stdlib.h>
@%:@include <string.h>
@%:@include <ql/quantlib.hpp>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.quantlibtest");
  */
  { FILE *fp = fopen("conf.quantlibtest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_quantlib_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_quantlib_version");
     exit(1);
   }

   if (($quantlib_major_version > major) ||
      (($quantlib_major_version == major) && ($quantlib_minor_version > minor)) ||
      (($quantlib_major_version == major) && ($quantlib_minor_version == minor) && ($quantlib_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'quantlib-config --version' returned %d.%d.%d, but the minimum version\n", $quantlib_major_version, $quantlib_minor_version, $quantlib_micro_version);
      printf("*** of QuantLib required is %d.%d.%d. If quantlib-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If quantlib-config was wrong, set the environment variable QUANTLIB_CONFIG\n");
      printf("*** to point to the correct copy of quantlib-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

      ],, no_quantlib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_quantlib" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$QUANTLIB_CONFIG" = "no" ; then
       echo "*** The quantlib-config script installed by QuantLib could not be found"
       echo "*** If QuantLib was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the QUANTLIB_CONFIG environment variable to the"
       echo "*** full path to quantlib-config."
     else
       if test -f conf.quantlibtest ; then
        :
       else
          echo "*** Could not run QuantLib test program, checking why..."
          CXXFLAGS="$CXXFLAGS $QUANTLIB_CXXFLAGS"
          LIBS="$LIBS $QUANTLIB_LIBS"
          AC_TRY_LINK([
@%:@include <stdio.h>
@%:@include <ql/quantlib.hpp>

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding QuantLib or finding the wrong"
          echo "*** version of QuantLib. If it is not finding QuantLib, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means QuantLib was incorrectly installed"
          echo "*** or that you have moved QuantLib since it was installed. In the latter case, you"
          echo "*** may want to edit the quantlib-config script: $QUANTLIB_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     QUANTLIB_CXXFLAGS=""
     QUANTLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(QUANTLIB_CXXFLAGS)
  AC_SUBST(QUANTLIB_LIBS)
  rm -f conf.quantlibtest
])
