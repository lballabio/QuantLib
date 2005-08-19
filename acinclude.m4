
# QL_CHECK_CONSTANT(NAME,TYPE,HEADER,VALUE,DESCRIPTION)
# ----------------------------------------------
# Check whether the constant NAME (of type TYPE) exists in HEADER.
# It defines it as VALUE if it cannot be found.
AC_DEFUN([QL_CHECK_CONSTANT],
[AC_MSG_CHECKING([for $1])
 AC_TRY_COMPILE(
    [@%:@include<$3>],
    [$2 x = $1;],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_DEFINE([$1],[$4],[$5])
    ])
])

# QL_CHECK_LONG_LONG
# ----------------------------------------------
# Check whether long long is supported.
AC_DEFUN([QL_CHECK_LONG_LONG],
[AC_MSG_CHECKING([long long support])
 AC_TRY_COMPILE(
    [],
    [long long i;
     unsigned long long j;
    ],
    [AC_MSG_RESULT([yes])
     AC_DEFINE([QL_HAVE_LONG_LONG],[],
               [Define this if your compiler supports the long long type.])
    ],
    [AC_MSG_RESULT([no])
    ])
])

# QL_CHECK_BOOST_DEVEL
# --------------------
# Check whether the Boost headers are available
AC_DEFUN([QL_CHECK_BOOST_DEVEL],
[AC_MSG_CHECKING([for Boost development files])
 AC_TRY_COMPILE(
    [@%:@include <boost/version.hpp>
     @%:@include <boost/shared_ptr.hpp>
     @%:@include <boost/assert.hpp>
     @%:@include <boost/current_function.hpp>],
    [],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([Boost development files not found])
    ])
])

# QL_CHECK_BOOST_VERSION
# ----------------------
# Check whether the Boost installation is up to date
AC_DEFUN([QL_CHECK_BOOST_VERSION],
[AC_MSG_CHECKING([Boost version])
 AC_REQUIRE([QL_CHECK_BOOST_DEVEL])
 AC_TRY_COMPILE(
    [@%:@include <boost/version.hpp>],
    [@%:@if BOOST_VERSION < 103100
     @%:@error too old
     @%:@endif],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([outdated Boost installation])
    ])
])

# QL_CHECK_BOOST_UNIT_TEST
# ------------------------
# Check whether the Boost unit-test framework is available
AC_DEFUN([QL_CHECK_BOOST_UNIT_TEST],
[AC_MSG_CHECKING([for Boost unit-test framework])
 AC_REQUIRE([AC_PROG_CC])
 ql_original_LIBS=$LIBS
 for boost_lib in boost_unit_test_framework-$CC boost_unit_test_framework ; do
     LIBS="$ql_original_LIBS -l$boost_lib"
     boost_unit_found=no
     AC_LINK_IFELSE(
         [@%:@include <boost/test/unit_test.hpp>
          using namespace boost::unit_test_framework;
          test_suite*
          init_unit_test_suite(int argc, char** argv)
          {
              return (test_suite*) 0;
          }
         ],
         [boost_unit_found=$boost_lib
          break],
         [])
 done
 LIBS="$ql_original_LIBS"
 if test "$boost_unit_found" = no ; then
     AC_MSG_RESULT([no])
     AC_SUBST([BOOST_UNIT_TEST_LIB],[""])
     AC_MSG_WARN([Boost unit-test framework not found])
     AC_MSG_WARN([The test suite will be disabled])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([BOOST_UNIT_TEST_LIB],[$boost_lib])
 fi
])

# QL_CHECK_BOOST_TEST_STREAM
# --------------------------
# Check whether Boost unit-test stream accepts std::fixed
AC_DEFUN([QL_CHECK_BOOST_TEST_STREAM],
[AC_MSG_CHECKING([whether Boost unit-test streams work])
 AC_REQUIRE([AC_PROG_CC])
 AC_TRY_COMPILE(
    [@%:@include <boost/test/unit_test.hpp>
     @%:@include <iomanip>],
    [BOOST_ERROR("foo " << std::fixed << 42.0);],
    [AC_MSG_RESULT([yes])
     AC_SUBST(BOOST_UNIT_TEST_DEFINE,[-DQL_WORKING_BOOST_STREAMS])],
    [AC_MSG_RESULT([no])
     AC_SUBST(BOOST_UNIT_TEST_DEFINE,[""])
    ])
])

# QL_CHECK_BOOST
# ------------------------
# Boost-related tests
AC_DEFUN([QL_CHECK_BOOST],
[AC_REQUIRE([QL_CHECK_BOOST_DEVEL])
 AC_REQUIRE([QL_CHECK_BOOST_VERSION])
 AC_REQUIRE([QL_CHECK_BOOST_UNIT_TEST])
 AC_REQUIRE([QL_CHECK_BOOST_TEST_STREAM])
])

