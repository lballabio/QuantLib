
# QL_CHECK_CPP14
# --------------------
# Check whether C++14 features are supported by default.
# If not (e.g., with Clang on Mac OS) add -std=c++14
AC_DEFUN([QL_CHECK_CPP14],
[AC_MSG_CHECKING([for C++14 support])
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[@%:@include <memory>
          class C {
            public:
              C(int) noexcept;
              auto f() { return std::make_unique<C>(1); }
          };
          ]],
        [[]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no: adding -std=c++14 to CXXFLAGS])
     AC_SUBST([CPP14_CXXFLAGS],["-std=c++14"])
     AC_SUBST([CXXFLAGS],["${CXXFLAGS} -std=c++14"])
    ])
])

# QL_CHECK_SYSTEM_HEADER_PREFIX
# -----------------------------
# Check whether the compiler supports the --system-header-prefix flag
AC_DEFUN([QL_CHECK_SYSTEM_HEADER_PREFIX],
[AC_MSG_CHECKING([if ${CXX} supports --system-header-prefix])
 AC_REQUIRE([AC_PROG_CC])
 ql_original_CXXFLAGS=$CXXFLAGS
 CXXFLAGS="$ql_original_CXXFLAGS --system-header-prefix=boost/"
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[int foo() { return 0; }]],
        [[]])],
    [AC_MSG_RESULT([yes])
     AC_SUBST([SYSTEM_HEADER_CXXFLAGS],["--system-header-prefix=boost/"])
    ],
    [AC_MSG_RESULT([no])])
 CXXFLAGS="$ql_original_CXXFLAGS ${SYSTEM_HEADER_CXXFLAGS}"
])

# QL_CHECK_BOOST_DEVEL
# --------------------
# Check whether the Boost headers are available
AC_DEFUN([QL_CHECK_BOOST_DEVEL],
[AC_MSG_CHECKING([for Boost development files])
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[@%:@include <boost/version.hpp>
          @%:@include <boost/shared_ptr.hpp>
          @%:@include <boost/assert.hpp>
          @%:@include <boost/current_function.hpp>]],
        [[]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([Boost development files not found])
    ])
])

# QL_CHECK_BOOST_VERSION
# ----------------------
# Check whether the Boost installation is up to date
AC_DEFUN([QL_CHECK_BOOST_VERSION],
[AC_MSG_CHECKING([for Boost version >= 1.48])
 AC_REQUIRE([QL_CHECK_BOOST_DEVEL])
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[@%:@include <boost/version.hpp>]],
        [[@%:@if BOOST_VERSION < 104800
          @%:@error too old
          @%:@endif]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([outdated Boost installation])
    ])
])

# QL_CHECK_BOOST_VERSION_1_58_OR_HIGHER
# ----------------------
# Check whether the Boost installation is version 1.58
AC_DEFUN([QL_CHECK_BOOST_VERSION_1_58_OR_HIGHER],
[AC_MSG_CHECKING([for Boost version >= 1.58])
 AC_REQUIRE([QL_CHECK_BOOST_DEVEL])
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[@%:@include <boost/version.hpp>]],
        [[@%:@if BOOST_VERSION < 105800
          @%:@error too old
          @%:@endif]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([Boost version 1.58 or higher is required for the thread-safe observer pattern])
    ])
])

# QL_CHECK_BOOST_VERSION_1_59_OR_HIGHER
# ----------------------
# Check whether the Boost installation is version 1.59
AC_DEFUN([QL_CHECK_BOOST_VERSION_1_59_OR_HIGHER],
[AC_MSG_CHECKING([for Boost version >= 1.59])
 AC_REQUIRE([QL_CHECK_BOOST_DEVEL])
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[@%:@include <boost/version.hpp>]],
        [[@%:@if BOOST_VERSION < 105900
          @%:@error too old
          @%:@endif]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([Boost version 1.59 or higher is required for the parallel unit test runner.])
    ])
])

# QL_CHECK_BOOST_UNIT_TEST
# ------------------------
# Check whether the Boost unit-test framework is available
AC_DEFUN([QL_CHECK_BOOST_UNIT_TEST],
[AC_MSG_CHECKING([for Boost.Test])
 AC_REQUIRE([AC_PROG_CC])
 ql_original_CXXFLAGS=$CXXFLAGS
 for test_flags in "" \
                   "-DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK" ; do
     # static version
     CXXFLAGS="$ql_original_CXXFLAGS $test_flags"
     boost_unit_found=no
     AC_LINK_IFELSE([AC_LANG_SOURCE(
         [@%:@include <boost/test/included/unit_test.hpp>
          using namespace boost::unit_test_framework;
          test_suite*
          init_unit_test_suite(int argc, char** argv)
          {
              return (test_suite*) 0;
          }
         ])],
         [boost_unit_found=yes
          boost_defines=$test_flags
          break],
         [])
 done
 CXXFLAGS="$ql_original_CXXFLAGS"
 if test "$boost_unit_found" = no ; then
     AC_MSG_RESULT([no])
     AC_SUBST([HAVE_BOOST_TEST],[""])
     AC_SUBST([BOOST_UNIT_TEST_MAIN_CXXFLAGS],[""])
     AC_MSG_WARN([Boost unit-test framework not found.])
     AC_MSG_WARN([The test suite will be disabled.])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([HAVE_BOOST_TEST],["yes"])
     AC_SUBST([BOOST_UNIT_TEST_MAIN_CXXFLAGS],[$boost_defines])
 fi
])

# QL_CHECK_BOOST_SIGNALS2
# ------------------------
# Check whether Boost signals2 is available
AC_DEFUN([QL_CHECK_BOOST_SIGNALS2],
[AC_MSG_CHECKING([whether Boost signals2 is available])
 AC_REQUIRE([AC_PROG_CC])
 ql_original_LIBS=$LIBS
 ql_original_CXXFLAGS=$CXXFLAGS
 LIBS="$ql_original_LIBS -pthread"
 CXXFLAGS="$ql_original_CXXFLAGS -pthread"
 boost_signals2_found=no
 AC_LINK_IFELSE([AC_LANG_SOURCE(
     [@%:@include <boost/signals2/signal.hpp>

      int main() {
        boost::signals2::signal<void()> sig;
      }
     ])],
     [boost_signals2_found=yes
      break],
     [])
 LIBS="$ql_original_LIBS"
 CXXFLAGS="$ql_original_CXXFLAGS"
     
 if test "$boost_signals2_found" = no ; then
     AC_MSG_RESULT([no])
     AC_SUBST([PTHREAD_LIB],[""])
     AC_MSG_ERROR([Boost signals2 library not found.
         This library is required by the thread-safe observer pattern.])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([PTHREAD_LIB],["-pthread"])
     AC_SUBST([PTHREAD_CXXFLAGS],["-pthread"])
     AC_SUBST([CXXFLAGS],["${CXXFLAGS} -pthread"])
 fi
])

# QL_CHECK_BOOST_TEST_INTERPROCESS
# ------------------------
# Check whether the Boost interprocess is available
AC_DEFUN([QL_CHECK_BOOST_TEST_INTERPROCESS],
[AC_MSG_CHECKING([whether Boost interprocess is available])
 AC_REQUIRE([AC_PROG_CC])
 ql_original_LIBS=$LIBS
 for boost_interprocess_lib in "-pthread" "-pthread -lrt"; do
     LIBS="$ql_original_LIBS $boost_interprocess_lib"
     boost_interprocess_found=no
        
	 AC_LINK_IFELSE([AC_LANG_SOURCE(
         [@%:@include <boost/interprocess/ipc/message_queue.hpp>
 
          using namespace boost::interprocess;
          int main() {
            message_queue mq(open_or_create,"message_queue",100,100);
            message_queue::remove("message_queue");
         
            return 0;
          }
         ])],
         [boost_interprocess_found=$boost_interprocess_lib
          break],
         [])
 done
 LIBS="$ql_original_LIBS"
     
 if test "$boost_interprocess_found" = no ; then
     AC_MSG_RESULT([no])
     AC_SUBST([BOOST_INTERPROCESS_LIB],[""])
     AC_MSG_ERROR([The boost library interprocess does not work on this system 
         but is required to use the parallel unit test runner.])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([BOOST_INTERPROCESS_LIB],[$boost_interprocess_lib])
     AC_SUBST([PTHREAD_CXXFLAGS],["-pthread"])
     AC_SUBST([CXXFLAGS],["${CXXFLAGS} -pthread"])
 fi
])
     

# QL_CHECK_BOOST
# ------------------------
# Boost-related tests
AC_DEFUN([QL_CHECK_BOOST],
[AC_REQUIRE([QL_CHECK_BOOST_DEVEL])
 AC_REQUIRE([QL_CHECK_BOOST_VERSION])
 AC_REQUIRE([QL_CHECK_BOOST_UNIT_TEST])
])

