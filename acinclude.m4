
# QL_TO_UPPER(STRING)
# -------------------
# Convert the passed string to uppercase
AC_DEFUN([QL_TO_UPPER],
[translit([$1],[abcdefghijklmnopqrstuvwxyz.],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_])])

# QL_CHECK_HEADER(HEADER)
# -----------------------
# Check whether <cheader> exists, falling back to <header.h>.
# It defines HAVE_CHEADER or HAVE_HEADER_H and sets the ql_header
# variable depending on the one found.
AC_DEFUN([QL_CHECK_HEADER],
[AC_CHECK_HEADER(
    [c$1],
    [AC_SUBST([ql_$1],[c$1])
     AC_DEFINE(QL_TO_UPPER([have_c$1]),[1],
               [Define to 1 if you have the <c$1> header file.])
    ],
    [AC_CHECK_HEADER([$1.h],
        [AC_SUBST([ql_$1],[$1.h])
         AC_DEFINE(QL_TO_UPPER(have_$1_h),1,
                   [Define to 1 if you have the <$1.h> header file.])
        ],
        [AC_MSG_ERROR([$1 not found])])
    ])
])

# QL_CHECK_FUNC(FUNCTION,ARGS,HEADER)
# -----------------------------------
# Check whether std::FUNCTION exists by calling it with arguments ARGS
# after including HEADER. It falls back on FUNCTION if it cannot be
# found in the std namespace. It defines QL_FUNCTION as the fully
# qualified name of the function.
AC_DEFUN([QL_CHECK_FUNC],
[AC_MSG_CHECKING([for $1])
 AC_TRY_COMPILE(
    [@%:@include <$3>],
    [double x;
     char s[10];
     std::$1($2);
    ],
    [AC_MSG_RESULT([yes])
     AC_DEFINE(QL_TO_UPPER([QL_$1]),[std::$1],
               [$1 function with full namespace specification])
    ],
    [AC_TRY_COMPILE(
        [@%:@include <$3>],
        [double x;
         char s[10];
         $1($2);
        ],
        [AC_MSG_RESULT([yes])
         AC_DEFINE(QL_TO_UPPER(QL_$1),[$1],
                   [$1 function with full namespace specification])
        ],
        [AC_MSG_RESULT([no])
         AC_MSG_ERROR([$1 not found])
        ])
    ])
])


# QL_CHECK_TYPE(TYPE,ARGS,HEADER)
# -------------------------------
# Check whether std::TYPE exists by instantiating it with arguments
# ARGS after including HEADER. It falls back on TYPE if it cannot be
# found in the std namespace. It defines QL_TYPE as the fully
# qualified name of the type.
AC_DEFUN([QL_CHECK_TYPE],
[AC_MSG_CHECKING([for $1])
 AC_TRY_COMPILE(
    [@%:@include <$3>],
    [std::$1 x = std::$1($2);],
    [AC_MSG_RESULT([yes])
     AC_DEFINE(QL_TO_UPPER(QL_$1),[std::$1],
               [$1 type with full namespace specification])
    ],
    [AC_TRY_COMPILE(
        [@%:@include <$3>],
        [$1 x = $1($2);],
        [AC_MSG_RESULT([yes])
         AC_DEFINE(QL_TO_UPPER(QL_$1),[$1],
                   [$1 type with full namespace specification])
        ],
        [AC_MSG_RESULT([no])
         AC_MSG_ERROR([$1 not found])
        ])
    ])
])


# QL_CHECK_TEMPLATE_TYPE(TYPE,TARGS,ARGS,HEADER)
# ----------------------------------------------
# Check whether std::TYPE<TARGS> exists by instantiating it with
# arguments ARGS after including HEADER. It falls back on TYPE<TARGS>
# if it cannot be found in the std namespace. It defines QL_TYPE as the
# fully qualified name of the type not including <TARGS>.
AC_DEFUN([QL_CHECK_TEMPLATE_TYPE],
[AC_MSG_CHECKING([for $1])
 AC_TRY_COMPILE(
    [@%:@include<$4>],
    [std::$1<$2> x = std::$1<$2>($3);],
    [AC_MSG_RESULT([yes])
     AC_DEFINE(QL_TO_UPPER(QL_$1),[std::$1],
               [$1 type with full namespace specification])
    ],
    [AC_TRY_COMPILE(
        [@%:@include<$4>],
        [$1<$2> x = $1<$2>($3);],
        [AC_MSG_RESULT([yes])
         AC_DEFINE(QL_TO_UPPER(QL_$1),[$1],
                   [$1 type with full namespace specification])
        ],
        [AC_MSG_RESULT([no])
         AC_MSG_ERROR([$1 not found])
        ])
    ])
])

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

# QL_CHECK_NAMESPACES
# ----------------------------------------------
# Check whether namespaces are supported.
AC_DEFUN([QL_CHECK_NAMESPACES],
[AC_MSG_CHECKING([namespace support])
 AC_TRY_COMPILE(
    [namespace Foo { struct A {}; }
     using namespace Foo;
    ],
    [A a;],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([namespaces not supported])
    ])
])

# QL_CHECK_GMTIME
# -------------------------------
# Check whether std::gmtime exists. It falls back on gmtime if it
# cannot be found in the std namespace. It defines QL_GMTIME as the
# fully qualified name of the function.
AC_DEFUN([QL_CHECK_GMTIME],
[AC_MSG_CHECKING([for gmtime])
 AC_TRY_COMPILE(
    [@%:@include <$ql_time>],
    [std::time_t t;
     std::time(&t);
     std::gmtime(&t);
    ],
    [AC_MSG_RESULT([yes])
     AC_DEFINE(QL_GMTIME,[std::gmtime],
               [gmtime function with full namespace specification])
    ],
    [AC_TRY_COMPILE(
        [@%:@include<$ql_time>],
        [time_t t;
         time(&t);
         gmtime(&t);
        ],
        [AC_MSG_RESULT([yes])
         AC_DEFINE(QL_GMTIME,[gmtime],
                   [gmtime function with full namespace specification])
        ],
        [AC_MSG_RESULT([no])
         AC_MSG_ERROR([gmtime not found])
        ])
    ])
])

# QL_CHECK_BOOST_DEVEL
# -----------------------
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
     AC_MSG_ERROR([Boost unit-test framework not found])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([BOOST_UNIT_TEST_LIB],[$boost_lib])
 fi
])
