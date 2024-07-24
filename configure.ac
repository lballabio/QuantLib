# Process this file with autoconf to produce a configure script.
AC_INIT([QuantLib], [1.36-dev],
        [quantlib-dev@lists.sourceforge.net],
        [QuantLib])
AC_PREREQ(2.62)
AC_CONFIG_SRCDIR([ql/qldefines.hpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([ql/config.hpp])
AM_INIT_AUTOMAKE([foreign nostdinc subdir-objects tar-ustar])
AC_CONFIG_MACRO_DIR([m4])

AC_COPYRIGHT([
Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 StatPro Italia srl
Copyright (C) 2004, 2005 Ferdinando Ametrano

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.])

AC_PROG_AWK
AC_PROG_INSTALL

# Help the compiler find external software

AC_ARG_WITH([boost-include],
            AS_HELP_STRING([--with-boost-include=INCLUDE_PATH],
                           [Supply the location of Boost header files]),
            [ql_boost_include_path="`cd ${withval} 2>/dev/null && pwd`"],
            [ql_boost_include_path=""])
if test [ -n "$ql_boost_include_path" ] ; then
   AC_SUBST([BOOST_INCLUDE],["-isystem ${ql_boost_include_path}"])
   AC_SUBST([CPPFLAGS],["${CPPFLAGS} -isystem ${ql_boost_include_path}"])
fi

# Continue setup

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
LT_INIT

AM_CONDITIONAL(ENABLE_STATIC, test "${enable_static}" == "yes")

AC_LANG([C++])

AC_ARG_ENABLE([openmp],
              AS_HELP_STRING([--enable-openmp],
                             [If enabled, configure will try to detect
                              and enable OpenMP support.]),
              [ql_openmp=$enableval],
              [ql_openmp=no])
if test "$ql_openmp" = "yes" ; then
   AC_OPENMP
   AC_SUBST([CXXFLAGS],["${CXXFLAGS} ${OPENMP_CXXFLAGS}"])
fi

# Check for C++14 support
QL_CHECK_CPP14

# Check for other compiler flags
QL_CHECK_SYSTEM_HEADER_PREFIX

# Check for Boost components
QL_CHECK_BOOST

# Check for Emacs
AM_PATH_LISPDIR
AM_CONDITIONAL(EMACS_FOUND, test "${EMACS}" != "no")

# Check for tools needed for building documentation
AC_PATH_PROG([DOXYGEN], [doxygen])
AC_PATH_PROG([SED], [sed])
AC_PATH_PROG([LATEX], [latex])
AC_PATH_PROG([PDFLATEX], [pdflatex])
AC_PATH_PROG([MAKEINDEX], [makeindex])
AC_PATH_PROG([DVIPS], [dvips])


# site configuration

# AC_ARG_WITH([real],
#             AS_HELP_STRING([--with-real=type],
#                            [The given type will be used for floating-point
#                             calculations.
#                             In case this option is not invoked, double
#                             will be used.]),
#             [AC_DEFINE_UNQUOTED([QL_REAL],[$withval],
#                                 [Define this as the type to be used for
#                                  floating-point calculations.])],
#             [AC_DEFINE_UNQUOTED([QL_REAL],[double],
#                                 [Define this as the type to be used for
#                                  floating-point calculations.])])

# AC_ARG_WITH([integer],
#             AS_HELP_STRING([--with-integer=type],
#                            [The given type will be used for integer
#                             calculations. At least an int is required.
#                             In case this option is not invoked, int
#                             will be used.]),
#             [AC_DEFINE_UNQUOTED([QL_INTEGER],[$withval],
#                                 [Define this as the type to be used for
#                                  integer calculations. At least an int
#                                  is required.])],
#             [AC_DEFINE_UNQUOTED([QL_INTEGER],[int],
#                                 [Define this as the type to be used for
#                                  integer calculations. At least an int
#                                  is required.])])

# AC_ARG_WITH([big-integer],
#             AS_HELP_STRING([--with-big-integer=type],
#                            [The given type will be used for large integer
#                             calculations. At least 4 bytes are required.
#                             In case this option is not invoked, long
#                             will be used.]),
#             [AC_DEFINE_UNQUOTED([QL_BIG_INTEGER],[$withval],
#                                 [Define this as the type to be used for
#                                  large integer calculations. At least
#                                  4 bytes are required.])],
#             [AC_DEFINE_UNQUOTED([QL_BIG_INTEGER],[long],
#                                 [Define this as the type to be used for
#                                  large integer calculations. At least
#                                  4 bytes are required.])])


AC_ARG_ENABLE([error-lines],
              AS_HELP_STRING([--enable-error-lines],
                             [If enabled, file and line information is
                              added to the error messages thrown by
                              the library.]),
              [ql_error_lines=$enableval],
              [ql_error_lines=no])
AC_MSG_CHECKING([whether to add file and line information to errors])
if test "$ql_error_lines" = "yes" ; then
   AC_DEFINE([QL_ERROR_LINES],[1],
             [Define this if error messages should include file and line
              information.])
fi
AC_MSG_RESULT([$ql_error_lines])

AC_ARG_ENABLE([error-functions],
              AS_HELP_STRING([--enable-error-functions],
                             [If enabled, current function information
                              is added to the error messages thrown by
                              the library.]),
              [ql_error_functions=$enableval],
              [ql_error_functions=no])
AC_MSG_CHECKING([whether to add function information to errors])
if test "$ql_error_functions" = "yes" ; then
   AC_DEFINE([QL_ERROR_FUNCTIONS],[1],
             [Define this if error messages should include current function
              information.])
fi
AC_MSG_RESULT([$ql_error_functions])

AC_ARG_ENABLE([tracing],
              AS_HELP_STRING([--enable-tracing],
                             [If enabled, tracing messages might be emitted
                              by the library depending on run-time
                              settings. Enabling this option can degrade
                              performance.]),
              [ql_tracing=$enableval],
              [ql_tracing=no])
AC_MSG_CHECKING([whether to enable tracing])
if test "$ql_tracing" = "yes" ; then
   AC_DEFINE([QL_ENABLE_TRACING],[1],
             [Define this if tracing messages should allowed (whether they
              are actually emitted will depend on run-time settings.)])
fi
AC_MSG_RESULT([$ql_tracing])

AC_MSG_CHECKING([whether to enable indexed coupons])
AC_ARG_ENABLE([indexed-coupons],
              AS_HELP_STRING([--enable-indexed-coupons],
                             [If enabled, indexed coupons (see the
                              documentation) are used in floating legs.
                              If disabled (the default), par coupons
                              are used.]),
              [ql_indexed_coupon=$enableval],
              [ql_indexed_coupon=no])
if test "$ql_indexed_coupon" = "yes" ; then
   AC_DEFINE([QL_USE_INDEXED_COUPON],[1],
             [Define this to use indexed coupons instead of par coupons in
              floating legs.])
fi
AC_MSG_RESULT([$ql_indexed_coupon])

AC_MSG_CHECKING([whether to enable extra safety checks])
AC_ARG_ENABLE([extra-safety-checks],
              AS_HELP_STRING([--enable-extra-safety-checks],
                             [If enabled, extra run-time checks are added
                              to a few functions. This can prevent their
                              inlining and degrade performance.]),
              [ql_belt_and_braces=$enableval],
              [ql_belt_and_braces=no])
if test "$ql_belt_and_braces" = "yes" ; then
   AC_DEFINE([QL_EXTRA_SAFETY_CHECKS],[1],
             [Define this if extra safety checks should be performed.
              This can degrade performance.])
fi
AC_MSG_RESULT([$ql_belt_and_braces])

AC_MSG_CHECKING([whether to enable sessions])
AC_ARG_ENABLE([sessions],
              AS_HELP_STRING([--enable-sessions],
                             [If enabled, singletons will return
                              different instances for different
                              threads; in particular, this means that
                              the evaluation date, the stored index
                              fixings and any other settings will be
                              per-thread. Undefined by default.]),
              [ql_use_sessions=$enableval],
              [ql_use_sessions=no])
if test "$ql_use_sessions" = "yes" ; then
   AC_DEFINE([QL_ENABLE_SESSIONS],[1],
             [Define this if you want to enable sessions.])
fi
AC_MSG_RESULT([$ql_use_sessions])

AC_MSG_CHECKING([whether to enable thread-safe observer pattern])
AC_ARG_ENABLE([thread-safe-observer-pattern],
              AS_HELP_STRING([--enable-thread-safe-observer-pattern],
                             [If enabled, thread-safe version of the 
                              observer pattern will be used. You should 
                              enable it if you want to use QuantLib 
                              via the SWIG layer within the JVM or .NET 
                              eco system or any environment with an 
                              async garbage collector. C++-17 is required
                              if this option is used together with 
                              --enable-std-pointers]),
              [ql_use_tsop=$enableval],
              [ql_use_tsop=no])
AC_MSG_RESULT([$ql_use_tsop])
if test "$ql_use_tsop" = "yes" ; then
   AC_DEFINE([QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN],[1],
             [Define this if you want to enable 
              thread-safe observer pattern.])
fi

if test "$ql_use_sessions" = "yes" || test "$ql_use_tsop" = "yes"; then
   QL_CHECK_BOOST_VERSION_1_58_OR_HIGHER
   QL_CHECK_BOOST_SIGNALS2
else
   AC_SUBST([PTHREAD_LIB],[""])
fi

AC_MSG_CHECKING([whether to enable parallel unit test runner])
AC_ARG_ENABLE([parallel-unit-test-runner],
              AS_HELP_STRING([--enable-parallel-unit-test-runner],
                             [If enabled, a parallel unit test runner is used 
                              to execute the C++ test suite. This will reduce 
                              the runtime on multi core CPUs.]),
              [ql_use_parallel_test=$enableval],
              [ql_use_parallel_test=no])
AC_MSG_RESULT([$ql_use_parallel_test])
if test "$ql_use_parallel_test" = "yes" ; then
   AC_DEFINE([QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER],[1],
             [Define this if you want to enable 
              the parallel unit test runner.])
   QL_CHECK_BOOST_VERSION_1_59_OR_HIGHER
   QL_CHECK_BOOST_TEST_INTERPROCESS
else
   AC_SUBST([BOOST_INTERPROCESS_LIB],[""])
fi

AC_MSG_CHECKING([whether to install examples])
AC_ARG_ENABLE([examples],
              AS_HELP_STRING([--enable-examples],
                             [If enabled, examples are built and installed
                              when "make" and "make install" are invoked.
                              If disabled (the default) they are built
                              but not installed.]),
              [ql_install_examples=$enableval],
              [ql_install_examples=no])
AM_CONDITIONAL(AUTO_EXAMPLES, test "$ql_install_examples" != "no")
AC_MSG_RESULT([$ql_install_examples])

AC_MSG_CHECKING([whether to skip examples])
AC_ARG_ENABLE([skip-examples],
              AS_HELP_STRING([--enable-skip-examples],
                             [If enabled, examples are not built.
                              Disabled by default.]),
              [ql_skip_examples=$enableval],
              [ql_skip_examples=no])
AM_CONDITIONAL(AUTO_SKIP_EXAMPLES, test "$ql_skip_examples" != "no")
AC_MSG_RESULT([$ql_skip_examples])

AC_MSG_CHECKING([whether to install the benchmark])
AC_ARG_ENABLE([benchmark],
              AS_HELP_STRING([--enable-benchmark],
                             [If enabled, the benchmark is built and
                              installed when "make" and "make install"
                              are invoked. If disabled (the default)
                              it is built but not installed.]),
              [ql_install_benchmark=$enableval],
              [ql_install_benchmark=no])
AM_CONDITIONAL(AUTO_BENCHMARK, test "$ql_install_benchmark" != "no")
AC_MSG_RESULT([$ql_install_benchmark])

AC_MSG_CHECKING([whether to use unity build])
AC_ARG_ENABLE([unity-build],
              AS_HELP_STRING([--enable-unity-build],
                             [If enabled, the source files in each directory
                              are collected into one single source file and
                              compiled together. This can speed up the
                              compilation of the library.
                              If disabled (the default) each source file is
                              compiled separately..]),
              [ql_use_unity_build=$enableval],
              [ql_use_unity_build=no])
AM_CONDITIONAL(UNITY_BUILD, test "$ql_use_unity_build" != "no")
AC_MSG_RESULT([$ql_use_unity_build])

AC_MSG_CHECKING([whether to enable intraday date and daycounters])
AC_ARG_ENABLE([intraday],
              AS_HELP_STRING([--enable-intraday],
                             [If enabled, date objects will support an intraday 
                              datetime resolution down to microseconds. 
                              Strickly monotone daycounters (Actual360, 
                              Actual365Fixed and ActualActual) will take 
                              the additional information into account 
                              and allow for accurate intraday pricing. 
                              If disabled (the default) the smallest 
                              resolution of date objects will be a single day.
                              Intraday datetime resolution is experimental.]),
              [ql_use_intraday=$enableval],
              [ql_use_intraday=no])
if test "$ql_use_intraday" = "yes" ; then
   AC_DEFINE([QL_HIGH_RESOLUTION_DATE],[1],
             [Define this if you want to enable high resolution date class.])
fi
AC_MSG_RESULT([$ql_use_intraday])

AC_MSG_CHECKING([whether to throw exceptions for notification cycles])
AC_ARG_ENABLE([throwing-in-cycles],
              AS_HELP_STRING([--enable-throwing-in-cycles],
                             [If enabled, lazy objects will raise an
                              exception when they detect a
                              notification cycle which would result in
                              an infinite recursion loop. If disabled
                              (the default), they will break the
                              recursion without throwing.  Enabling
                              this option is recommended but might
                              cause existing code to throw.]),
              [ql_throw_in_cycles=$enableval],
              [ql_throw_in_cycles=no])
if test "$ql_throw_in_cycles" = "yes" ; then
   AC_DEFINE([QL_THROW_IN_CYCLES],[1],
             [Define this if you want to throw an exception when a notification loop is detected.])
fi
AC_MSG_RESULT([$ql_throw_in_cycles])

AC_MSG_CHECKING([whether to reduce the number of notifications from lazy objects])
AC_ARG_ENABLE([faster-lazy-objects],
              AS_HELP_STRING([--enable-faster-lazy-objects],
                             [If enabled, lazy objects will forward
                              the first notification received, and
                              discard the others until recalculated;
                              the rationale is that observers were
                              already notified, and don't need further
                              notifications until they recalculate, at
                              which point this object would be
                              recalculated too.  After recalculation,
                              this object would again forward the
                              first notification received.  Although
                              not always correct, this behavior is a
                              lot faster and thus is the current
                              default.]),
              [ql_faster_lazy_objects=$enableval],
              [ql_faster_lazy_objects=yes])
if test "$ql_faster_lazy_objects" = "yes" ; then
   AC_DEFINE([QL_FASTER_LAZY_OBJECTS],[1],
             [Define this if you want lazy objects to forward fewer notifications.])
fi
AC_MSG_RESULT([$ql_faster_lazy_objects])

AC_MSG_CHECKING([whether to enable std::any instead of boost::any])
AC_ARG_ENABLE([std-any],
              AS_HELP_STRING([--enable-std-any],
                             [If enabled, std::any and related classes and
                              functions will be used instead of boost::any.
                              This requires C++17. If disabled (the default)
                              the Boost facilities are used.]),
              [ql_use_std_any=$enableval],
              [ql_use_std_any=no])
if test "$ql_use_std_any" = "yes" ; then
   AC_DEFINE([QL_USE_STD_ANY],[1],
             [Define this if you want to use std::any.])
fi
AC_MSG_RESULT([$ql_use_std_any])

AC_MSG_CHECKING([whether to enable std::optional instead of boost::optional])
AC_ARG_ENABLE([std-optional],
              AS_HELP_STRING([--enable-std-optional],
                             [If enabled, std::optional and related
                              classes and functions will be used instead
                              of boost::optional. This requires C++17.
                              If disabled (the default) the Boost facilities
                              are used.]),
              [ql_use_std_optional=$enableval],
              [ql_use_std_optional=no])
if test "$ql_use_std_optional" = "yes" ; then
   AC_DEFINE([QL_USE_STD_OPTIONAL],[1],
             [Define this if you want to use std::optional.])
fi
AC_MSG_RESULT([$ql_use_std_optional])

AC_MSG_CHECKING([whether to enable standard smart pointers])
AC_ARG_ENABLE([std-pointers],
              AS_HELP_STRING([--enable-std-pointers],
                             [If enabled, std::shared_ptr and related
                              classes and functions will be used instead
                              of boost::shared_ptr.
                              If disabled (the default) the Boost facilities
                              are used.
                              Note that std::shared_ptr does not check
                              access and can cause segmentation faults.]),
              [ql_use_std_pointers=$enableval],
              [ql_use_std_pointers=no])
if test "$ql_use_std_pointers" = "yes" ; then
   AC_DEFINE([QL_USE_STD_SHARED_PTR],[1],
             [Define this if you want to use standard smart pointers.])
fi
AC_MSG_RESULT([$ql_use_std_pointers])

AC_MSG_CHECKING([whether to enable available std classes])
AC_ARG_ENABLE([std-classes],
              AS_HELP_STRING([--enable-std-classes],
                             [This is a shortcut for --enable-std-pointers
                              --enable-std-function --enable-std-tuple.
                              If enabled, this supersedes any --disable
                              option passed for the above.]),
              [ql_use_std_classes=$enableval],
              [ql_use_std_classes=no])
if test "$ql_use_std_classes" = "yes" ; then
   AC_DEFINE([QL_USE_STD_SHARED_PTR],[1],
             [Define this if you want to use standard smart pointers.])
fi
AC_MSG_RESULT([$ql_use_std_classes])

AC_MSG_CHECKING([whether to enable the implementation of Null as template functions])
AC_ARG_ENABLE([null-as-functions],
              AS_HELP_STRING([--enable-null-as-functions],
                             [If enabled, Null will be implemented as a set
                              of template functions.  This allows the code
                              to work with user-defined Real types but was
                              reported to cause internal compiler errors
                              with Visual C++ 2022 in some cases.
                              If disabled (the default) Null will be
                              implemented as a class template, as in
                              previous releases.]),
              [ql_use_null_functions=$enableval],
              [ql_use_null_functions=no])
if test "$ql_use_null_functions" = "yes" ; then
   AC_DEFINE([QL_NULL_AS_FUNCTIONS],[1],
             [Define this if you want to enable the implementation of Null as template functions.])
fi
AC_MSG_RESULT([$ql_use_null_functions])

AC_MSG_CHECKING([whether to compile the test suite])
AC_ARG_ENABLE([test-suite],
              AS_HELP_STRING([--enable-test-suite],
                             [If enabled (the default), the test suite is
                              also built, assuming Boost.Test could be found.
                              If disabled, it is not. Do not try this at home.]),
              [ql_build_test_suite=$enableval],
              [ql_build_test_suite=yes])
AM_CONDITIONAL(BUILD_TEST_SUITE, test "$ql_build_test_suite" != "no" && test "x${HAVE_BOOST_TEST}" != "x" )
AC_MSG_RESULT([$ql_build_test_suite])


# manual configurations for specific hosts
case $host in
  sparc-sun-solaris*)
    AC_DEFINE([QL_PATCH_SOLARIS],[1],
              [Define if running on a Sun Solaris machine.])
    ;;
esac


# done, output the configured files
AC_CONFIG_FILES([
    Makefile
    ql/Makefile
    ql/cashflows/Makefile
    ql/currencies/Makefile
    ql/experimental/Makefile
    ql/experimental/asian/Makefile
    ql/experimental/averageois/Makefile
    ql/experimental/barrieroption/Makefile
    ql/experimental/basismodels/Makefile
    ql/experimental/callablebonds/Makefile
    ql/experimental/catbonds/Makefile
    ql/experimental/commodities/Makefile
    ql/experimental/coupons/Makefile
    ql/experimental/credit/Makefile
    ql/experimental/exoticoptions/Makefile
    ql/experimental/finitedifferences/Makefile
    ql/experimental/forward/Makefile
    ql/experimental/fx/Makefile
    ql/experimental/inflation/Makefile
    ql/experimental/lattices/Makefile
    ql/experimental/math/Makefile
    ql/experimental/mcbasket/Makefile
    ql/experimental/models/Makefile
    ql/experimental/processes/Makefile
    ql/experimental/risk/Makefile
    ql/experimental/shortrate/Makefile
    ql/experimental/swaptions/Makefile
    ql/experimental/termstructures/Makefile
    ql/experimental/variancegamma/Makefile
    ql/experimental/varianceoption/Makefile
    ql/experimental/volatility/Makefile
    ql/indexes/Makefile
    ql/indexes/ibor/Makefile
    ql/indexes/inflation/Makefile
    ql/indexes/swap/Makefile
    ql/instruments/Makefile
    ql/instruments/bonds/Makefile
    ql/legacy/Makefile
    ql/legacy/libormarketmodels/Makefile
    ql/math/Makefile
    ql/math/copulas/Makefile
    ql/math/distributions/Makefile
    ql/math/integrals/Makefile
    ql/math/interpolations/Makefile
    ql/math/matrixutilities/Makefile
    ql/math/ode/Makefile
    ql/math/optimization/Makefile
    ql/math/randomnumbers/Makefile
    ql/math/solvers1d/Makefile
    ql/math/statistics/Makefile
    ql/methods/Makefile
    ql/methods/finitedifferences/Makefile    
    ql/methods/finitedifferences/meshers/Makefile
    ql/methods/finitedifferences/operators/Makefile
    ql/methods/finitedifferences/schemes/Makefile
    ql/methods/finitedifferences/solvers/Makefile
    ql/methods/finitedifferences/stepconditions/Makefile
    ql/methods/finitedifferences/utilities/Makefile
    ql/methods/lattices/Makefile
    ql/methods/montecarlo/Makefile
    ql/models/Makefile
    ql/models/equity/Makefile
    ql/models/marketmodels/Makefile
    ql/models/marketmodels/browniangenerators/Makefile
    ql/models/marketmodels/callability/Makefile
    ql/models/marketmodels/correlations/Makefile
    ql/models/marketmodels/curvestates/Makefile
    ql/models/marketmodels/driftcomputation/Makefile
    ql/models/marketmodels/evolvers/Makefile
    ql/models/marketmodels/evolvers/volprocesses/Makefile
    ql/models/marketmodels/models/Makefile
    ql/models/marketmodels/pathwisegreeks/Makefile
    ql/models/marketmodels/products/Makefile
    ql/models/marketmodels/products/onestep/Makefile
    ql/models/marketmodels/products/multistep/Makefile
    ql/models/marketmodels/products/pathwise/Makefile
    ql/models/shortrate/Makefile
    ql/models/shortrate/calibrationhelpers/Makefile
    ql/models/shortrate/onefactormodels/Makefile
    ql/models/shortrate/twofactormodels/Makefile
    ql/models/volatility/Makefile
    ql/patterns/Makefile
    ql/pricingengines/Makefile
    ql/pricingengines/asian/Makefile
    ql/pricingengines/barrier/Makefile
    ql/pricingengines/basket/Makefile
    ql/pricingengines/bond/Makefile
    ql/pricingengines/capfloor/Makefile
    ql/pricingengines/cliquet/Makefile
    ql/pricingengines/credit/Makefile
    ql/pricingengines/exotic/Makefile
    ql/pricingengines/forward/Makefile
    ql/pricingengines/inflation/Makefile
    ql/pricingengines/lookback/Makefile
    ql/pricingengines/quanto/Makefile
    ql/pricingengines/swap/Makefile
    ql/pricingengines/swaption/Makefile
    ql/pricingengines/vanilla/Makefile
    ql/processes/Makefile
    ql/quotes/Makefile
    ql/termstructures/Makefile
    ql/termstructures/credit/Makefile
    ql/termstructures/inflation/Makefile
    ql/termstructures/volatility/Makefile
    ql/termstructures/volatility/equityfx/Makefile
    ql/termstructures/volatility/capfloor/Makefile
    ql/termstructures/volatility/inflation/Makefile
    ql/termstructures/volatility/optionlet/Makefile
    ql/termstructures/volatility/swaption/Makefile
    ql/termstructures/yield/Makefile
    ql/time/Makefile
    ql/time/calendars/Makefile
    ql/time/daycounters/Makefile
    ql/utilities/Makefile
    man/Makefile
    m4/Makefile
    Docs/Makefile
    Examples/Makefile
    test-suite/Makefile])
AC_CONFIG_FILES([quantlib-config],[chmod +x quantlib-config])
AC_CONFIG_FILES([QuantLib.spec])
AC_CONFIG_FILES([quantlib.pc])
AC_OUTPUT

