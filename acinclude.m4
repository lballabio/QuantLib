
# QL_CHECK_CPP11
# --------------------
# Check whether C++11 features are supported by default.
# If not (e.g., with Clang on Mac OS) add -std=c++11
AC_DEFUN([QL_CHECK_CPP11],
[AC_MSG_CHECKING([for C++11 support])
 AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[@%:@include <initializer_list>
          struct S {
            int i = 3;
            double x = 3.5;
          };

          class C {
            public:
              C(int) noexcept;
              C(std::initializer_list<int>);
              S f() { return { 2, 1.5 }; }
          };
          ]],
        [[]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no: adding -std=c++11 to CXXFLAGS])
     AC_SUBST([CPP11_CXXFLAGS],["-std=c++11"])
     AC_SUBST([CXXFLAGS],["${CXXFLAGS} -std=c++11"])
    ])
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
 ql_original_LIBS=$LIBS
 ql_original_CXXFLAGS=$CXXFLAGS
 CC_BASENAME=`basename $CC`
 CC_VERSION=`echo "__GNUC__ __GNUC_MINOR__" | $CC -E -x c - | tail -n 1 | $SED -e "s/ //"`
 for suffix in "" \
               "-$CC_BASENAME$CC_VERSION" \
               "-$CC_BASENAME" \
               "-mt-$CC_BASENAME$CC_VERSION" \
               "-$CC_BASENAME$CC_VERSION-mt" \
               "-x$CC_BASENAME$CC_VERSION-mt" \
               "-mt-$CC_BASENAME" \
               "-$CC_BASENAME-mt" \
               "-mt" ; do
     boost_libs="-lboost_unit_test_framework$suffix"
     LIBS="$ql_original_LIBS $boost_libs"
     # static version
     CXXFLAGS="$ql_original_CXXFLAGS"
     boost_unit_found=no
     AC_LINK_IFELSE([AC_LANG_SOURCE(
         [@%:@include <boost/test/unit_test.hpp>
          using namespace boost::unit_test_framework;
          test_suite*
          init_unit_test_suite(int argc, char** argv)
          {
              return (test_suite*) 0;
          }
         ])],
         [boost_unit_found=$boost_libs
          boost_defines=""
          break],
         [])
     # shared version
     CXXFLAGS="$ql_original_CXXFLAGS -DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK"
     boost_unit_found=no
     AC_LINK_IFELSE([AC_LANG_SOURCE(
         [@%:@include <boost/test/unit_test.hpp>
          using namespace boost::unit_test_framework;
          test_suite*
          init_unit_test_suite(int argc, char** argv)
          {
              return (test_suite*) 0;
          }
         ])],
         [boost_unit_found=$boost_libs
          boost_defines="-DBOOST_TEST_DYN_LINK"
          break],
         [])
 done
 LIBS="$ql_original_LIBS"
 CXXFLAGS="$ql_original_CXXFLAGS"
 if test "$boost_unit_found" = no ; then
     AC_MSG_RESULT([no])
     AC_SUBST([BOOST_UNIT_TEST_LIB],[""])
     AC_SUBST([BOOST_UNIT_TEST_MAIN_CXXFLAGS],[""])
     AC_MSG_WARN([Boost unit-test framework not found.])
     AC_MSG_WARN([The test suite will be disabled.])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([BOOST_UNIT_TEST_LIB],[$boost_libs])
     AC_SUBST([BOOST_UNIT_TEST_MAIN_CXXFLAGS],[$boost_defines])
 fi
])

# QL_CHECK_BOOST_TEST_THREAD_SIGNALS2_SYSTEM
# ------------------------
# Check whether the Boost thread and system is available
AC_DEFUN([QL_CHECK_BOOST_TEST_THREAD_SIGNALS2_SYSTEM],
[AC_MSG_CHECKING([whether Boost thread, signals2 and system are available])
 AC_REQUIRE([AC_PROG_CC])
 ql_original_LIBS=$LIBS
 ql_original_CXXFLAGS=$CXXFLAGS
 CC_BASENAME=`basename $CC`
 CC_VERSION=`echo "__GNUC__ __GNUC_MINOR__" | $CC -E -x c - | tail -n 1 | $SED -e "s/ //"`
 for suffix in "" \
               "-$CC_BASENAME$CC_VERSION" \
               "-$CC_BASENAME" \
               "-mt-$CC_BASENAME$CC_VERSION" \
               "-$CC_BASENAME$CC_VERSION-mt" \
               "-x$CC_BASENAME$CC_VERSION-mt" \
               "-mt-$CC_BASENAME" \
               "-$CC_BASENAME-mt" \
               "-mt" ; do
     boost_thread_lib="-lboost_thread$suffix -lboost_system$suffix"
     LIBS="$ql_original_LIBS $boost_thread_lib"
     CXXFLAGS="$ql_original_CXXFLAGS -pthread"
     boost_thread_found=no
     AC_LINK_IFELSE([AC_LANG_SOURCE(
         [@%:@include <boost/thread/locks.hpp>
          @%:@include <boost/thread/recursive_mutex.hpp>
          @%:@include <boost/signals2/signal.hpp>
          
          #ifndef BOOST_THREAD_PLATFORM_PTHREAD
          #error only pthread is supported on this plattform
          #endif
    
          int main() {
            boost::recursive_mutex m;
            boost::lock_guard<boost::recursive_mutex> lock(m);
      
            boost::signals2::signal<void()> sig;
      
            return 0;
          }
         ])],
         [boost_thread_found=$boost_thread_lib
          break],
         [])
 done
 LIBS="$ql_original_LIBS"
 CXXFLAGS="$ql_original_CXXFLAGS"
     
 if test "$boost_thread_found" = no ; then
     AC_MSG_RESULT([no])
     AC_SUBST([BOOST_THREAD_LIB],[""])
     AC_MSG_ERROR([Boost thread, signals2 and system libraries not found. 
         These libraries are required by the thread-safe observer pattern
         or by the parallel unit test runner.])
 else
     AC_MSG_RESULT([yes])
     AC_SUBST([BOOST_THREAD_LIB],[$boost_thread_lib])
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
 AC_REQUIRE([QL_CHECK_BOOST_TEST_THREAD_SIGNALS2_SYSTEM])
 ql_original_LIBS=$LIBS
 for boost_interprocess_lib in "" "-lrt"; do 
     LIBS="$ql_original_LIBS $boost_thread_lib $boost_interprocess_lib"
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

