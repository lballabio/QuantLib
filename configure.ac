# Process this file with autoconf to produce a configure script.
AC_INIT([QuantLib], [1.7],
        [quantlib-dev@lists.sourceforge.net],
        [QuantLib])
AC_PREREQ(2.62)
AC_CONFIG_SRCDIR([ql/qldefines.hpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([ql/config.hpp])
AM_INIT_AUTOMAKE([foreign])
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
            AC_HELP_STRING([--with-boost-include=INCLUDE_PATH],
                           [Supply the location of Boost header files]),
            [ql_boost_include_path="`cd ${withval} 2>/dev/null && pwd`"],
            [ql_boost_include_path=""])
if test [ -n "$ql_boost_include_path" ] ; then
   AC_SUBST([BOOST_INCLUDE],["-I${ql_boost_include_path}"])
   AC_SUBST([CPPFLAGS],["${CPPFLAGS} -I${ql_boost_include_path}"])
fi
AC_ARG_WITH([boost-lib],
            AC_HELP_STRING([--with-boost-lib=LIB_PATH],
                           [Supply the location of Boost libraries]),
            [ql_boost_lib_path="`cd ${withval} 2>/dev/null && pwd`"],
            [ql_boost_lib_path=""])
if test [ -n "$ql_boost_lib_path" ] ; then
   AC_SUBST([BOOST_LIB],["-L${ql_boost_lib_path}"])
   AC_SUBST([LDFLAGS],["${LDFLAGS} -L${ql_boost_lib_path}"])
fi


# Continue setup

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

AM_CONDITIONAL(ENABLE_STATIC, test "${enable_static}" == "yes")

AC_LANG([C++])

AC_ARG_ENABLE([openmp],
              AC_HELP_STRING([--enable-openmp],
                             [If enabled, configure will try to detect
                              and enable OpenMP support.]),
              [ql_openmp=$enableval],
              [ql_openmp=no])
if test "$ql_openmp" = "yes" ; then
   AC_OPENMP
   AC_SUBST([CXXFLAGS],["${CXXFLAGS} ${OPENMP_CXXFLAGS}"])
fi

# Check for mandatory features

QL_CHECK_ASINH

# Check for Boost components
QL_CHECK_BOOST
AM_CONDITIONAL(BOOST_UNIT_TEST_FOUND, test "x${BOOST_UNIT_TEST_LIB}" != "x")

# Check for optional features
QL_CHECK_LONG_LONG

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
#             AC_HELP_STRING([--with-real=type],
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
#             AC_HELP_STRING([--with-integer=type],
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
#             AC_HELP_STRING([--with-big-integer=type],
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
              AC_HELP_STRING([--enable-error-lines],
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
              AC_HELP_STRING([--enable-error-functions],
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
              AC_HELP_STRING([--enable-tracing],
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
              AC_HELP_STRING([--enable-indexed-coupons],
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

AC_MSG_CHECKING([whether to enable negative rates])
AC_ARG_ENABLE([negative-rates],
              AC_HELP_STRING([--enable-negative-rates],
                             [If enabled (the default), negative yield
                              rates are allowed.  If disabled, some
                              features (notably, curve bootstrapping)
                              will throw when negative rates are found.]),
              [ql_negative_rates=$enableval],
              [ql_negative_rates=yes])
if test "$ql_negative_rates" = "yes" ; then
   AC_DEFINE([QL_NEGATIVE_RATES],[1],
             [Define this if negative yield rates should be allowed.])
fi
AC_MSG_RESULT([$ql_negative_rates])

AC_MSG_CHECKING([whether to enable extra safety checks])
AC_ARG_ENABLE([extra-safety-checks],
              AC_HELP_STRING([--enable-extra-safety-checks],
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
              AC_HELP_STRING([--enable-sessions],
                             [If enabled, singletons will return different
                              instances for different sessions. You will
                              have to provide and link with the library
                              a sessionId() function in namespace QuantLib,
                              returning a different session id for each
                              session.]),
              [ql_use_sessions=$enableval],
              [ql_use_sessions=no])
if test "$ql_use_sessions" = "yes" ; then
   AC_DEFINE([QL_ENABLE_SESSIONS],[1],
             [Define this if you want to enable sessions.])
fi
AC_MSG_RESULT([$ql_use_sessions])

AC_MSG_CHECKING([whether to install examples])
AC_ARG_ENABLE([examples],
              AC_HELP_STRING([--enable-examples],
                             [If enabled, examples are built and installed
                              when "make" and "make install" are invoked.
                              If disabled (the default) they are built
                              but not installed.]),
              [ql_install_examples=$enableval],
              [ql_install_examples=no])
AM_CONDITIONAL(AUTO_EXAMPLES, test "$ql_install_examples" != "no")
AC_MSG_RESULT([$ql_install_examples])

AC_MSG_CHECKING([whether to install the benchmark])
AC_ARG_ENABLE([benchmark],
              AC_HELP_STRING([--enable-benchmark],
                             [If enabled, the benchmark is built and
                              installed when "make" and "make install"
                              are invoked. If disabled (the default)
                              it is built but not installed.]),
              [ql_install_benchmark=$enableval],
              [ql_install_benchmark=no])
AM_CONDITIONAL(AUTO_BENCHMARK, test "$ql_install_benchmark" != "no")
AC_MSG_RESULT([$ql_install_benchmark])


# manual configurations for specific hosts
case $host in
  powerpc-apple-darwin*)
    AC_DEFINE([QL_PATCH_DARWIN],[1],
              [Define if running on a Mac OS X machine.])
    ;;
  sparc-sun-solaris*)
    AC_DEFINE([QL_PATCH_SOLARIS],[1],
              [Define if running on a Sun Solaris machine.])
    ;;
esac


# done, output the configured files
AC_CONFIG_FILES([
    Makefile
    config/Makefile
    ql/Makefile
    ql/cashflows/Makefile
    ql/currencies/Makefile
    ql/experimental/Makefile
    ql/experimental/amortizingbonds/Makefile
    ql/experimental/barrieroption/Makefile
    ql/experimental/callablebonds/Makefile
    ql/experimental/catbonds/Makefile
    ql/experimental/commodities/Makefile
    ql/experimental/convertiblebonds/Makefile
    ql/experimental/coupons/Makefile
    ql/experimental/credit/Makefile
    ql/experimental/exoticoptions/Makefile
    ql/experimental/finitedifferences/Makefile
    ql/experimental/fx/Makefile
    ql/experimental/inflation/Makefile
    ql/experimental/lattices/Makefile
    ql/experimental/math/Makefile
    ql/experimental/mcbasket/Makefile
    ql/experimental/processes/Makefile
    ql/experimental/risk/Makefile
    ql/experimental/shortrate/Makefile
    ql/experimental/swaptions/Makefile
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
    Examples/BasketLosses/Makefile
    Examples/BermudanSwaption/Makefile
    Examples/Bonds/Makefile
    Examples/CallableBonds/Makefile
    Examples/CDS/Makefile
    Examples/ConvertibleBonds/Makefile
    Examples/DiscreteHedging/Makefile
    Examples/EquityOption/Makefile
    Examples/FittedBondCurve/Makefile
    Examples/FRA/Makefile
    Examples/LatentModel/Makefile
    Examples/Gaussian1dModels/Makefile
    Examples/MarketModels/Makefile
    Examples/MultidimIntegral/Makefile
    Examples/Replication/Makefile
    Examples/Repo/Makefile
    Examples/Swap/Makefile
    test-suite/Makefile])
AC_CONFIG_FILES([quantlib-config],[chmod +x quantlib-config])
AC_CONFIG_FILES([QuantLib.spec])
AC_OUTPUT
