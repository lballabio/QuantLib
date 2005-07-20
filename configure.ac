# Process this file with autoconf to produce a configure script.
AC_INIT([QuantLib], [0.3.11],
        [quantlib-dev@lists.sourceforge.net],
        [QuantLib])
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([ql/qldefines.hpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([ql/config.hpp])
AM_INIT_AUTOMAKE([foreign])

AC_COPYRIGHT([Copyright (C) 2004, 2005 Ferdinando Ametrano
Copyright (C) 2000-2005 StatPro Italia srl

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/reference/license.html>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.])

AC_PROG_AWK
AC_PROG_INSTALL

# Default compilation variables

AC_SUBST([CXXFLAGS],[${CXXFLAGS='-g -O2 -Wall'}])

# Help the compiler find external software

AC_ARG_WITH([boost-include],
            AC_HELP_STRING([--with-boost-include=INCLUDE_PATH],
                           [Supply the location of Boost header files]),
            [ql_boost_include_path=$withval],
            [ql_boost_include_path=""])
if test [ -n "$ql_boost_include_path" ] ; then
   AC_SUBST([CPPFLAGS],["${CPPFLAGS} -I${ql_boost_include_path}"])
fi
AC_ARG_WITH([boost-lib],
            AC_HELP_STRING([--with-boost-lib=LIB_PATH],
                           [Supply the location of Boost libraries]),
            [ql_boost_lib_path=$withval],
            [ql_boost_lib_path=""])
if test [ -n "$ql_boost_lib_path" ] ; then
   AC_SUBST([LDFLAGS],["${LDFLAGS} -L${ql_boost_lib_path}"])
fi



# Continue setup

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_LANG([C++])

# Check for mandatory features and header files

QL_CHECK_NAMESPACES

QL_CHECK_HEADER([stdlib])

QL_CHECK_HEADER([math])
QL_CHECK_CONSTANT([M_PI],[double],[$ql_math],
                  [3.141592653589793238462643383280],[pi])
QL_CHECK_CONSTANT([M_SQRT_2],[double],[$ql_math],
                  [0.7071067811865475244008443621048],[1/sqrt(2)])
QL_CHECK_CONSTANT([M_SQRTPI],[double],[$ql_math],
                  [1.77245385090551602792981],[sqrt(pi)])
QL_CHECK_CONSTANT([M_1_SQRTPI],[double],[$ql_math],
                  [0.564189583547756286948],[1/sqrt(pi)])

QL_CHECK_HEADER([ctype])
QL_CHECK_HEADER([time])

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
if test "$ql_error_lines" = "yes" ; then
   AC_DEFINE([QL_ERROR_LINES],[1],
             [Define this if error messages should include file and line
              information.])
fi

AC_ARG_ENABLE([error-functions],
              AC_HELP_STRING([--enable-error-functions],
                             [If enabled, current function information
                              is added to the error messages thrown by
                              the library.]),
              [ql_error_functions=$enableval],
              [ql_error_functions=no])
if test "$ql_error_functions" = "yes" ; then
   AC_DEFINE([QL_ERROR_FUNCTIONS],[1],
             [Define this if error messages should include current function
              information.])
fi

AC_ARG_ENABLE([tracing],
              AC_HELP_STRING([--enable-tracing],
                             [If enabled, tracing messages might be emitted
                              by the library depending on run-time
                              settings. Enabling this option can degrade
                              performance.]),
              [ql_tracing=$enableval],
              [ql_tracing=no])
if test "$ql_tracing" = "yes" ; then
   AC_DEFINE([QL_ENABLE_TRACING],[1],
             [Define this if tracing messages should allowed (whether they
              are actually emitted will depend on run-time settings.)])
fi

AC_ARG_ENABLE([indexed-coupons],
              AC_HELP_STRING([--enable-indexed-coupons],
                             [If enabled, indexed coupons (see the
                              documentation) are used in floating legs.
                              If disabled (the default,) par coupons
                              are used.]),
              [ql_indexed_coupon=$enableval],
              [ql_indexed_coupon=no])
if test "$ql_indexed_coupon" = "yes" ; then
   AC_DEFINE([QL_USE_INDEXED_COUPON],[1],
             [Define this to use indexed coupons instead of par coupons in
              floating legs.])
fi

AC_ARG_ENABLE([negative-rates],
              AC_HELP_STRING([--enable-negative-rates],
                             [If enabled, negative yield rates are allowed
                              in a few places where they are currently
                              forbidden. It is still not clear whether
                              this is safe.]),
              [ql_negative_rates=$enableval],
              [ql_negative_rates=no])
if test "$ql_negative_rates" = "yes" ; then
   AC_DEFINE([QL_NEGATIVE_RATES],[1],
             [Define this if negative yield rates should be allowed.
              This might not be safe.])
fi

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

AC_ARG_ENABLE([early-payments],
              AC_HELP_STRING([--enable-early-payments],
                             [If enabled (the default,) payments are
                              considered to be settled at the beginning
                              of the day. Therefore, payments occurring
                              at today's date are not included in the NPV
                              of an instrument.]),
              [ql_early_payments=$enableval],
              [ql_early_payments=yes])
if test "$ql_early_payments" = "no" ; then
   AC_DEFINE([QL_TODAYS_PAYMENTS],[1],
             [Define this if payments occurring today should enter the NPV
              of an instrument.])
fi

AC_ARG_ENABLE([deprecated-code],
              AC_HELP_STRING([--enable-deprecated-code],
                             [If enabled (the default,) deprecated code
                              will be included in the library.]),
              [ql_use_deprecated_code=$enableval],
              [ql_use_deprecated_code=yes])
if test "$ql_use_deprecated_code" = "no" ; then
   AC_DEFINE([QL_DISABLE_DEPRECATED],[1],
             [Define this if you want to disable deprecated code.])
fi

AC_ARG_ENABLE([examples],
              AC_HELP_STRING([--enable-examples],
                             [If enabled, examples are built and installed
                              when "make" and "make install" are invoked.
                              If disabled (the default) they are built
                              with "make examples" and not installed.]),
              [ql_install_examples=$enableval],
              [ql_install_examples=no])
AM_CONDITIONAL(AUTO_EXAMPLES, test "$ql_install_examples" != "no")


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
    ql/Calendars/Makefile
    ql/CashFlows/Makefile
    ql/Currencies/Makefile
    ql/DayCounters/Makefile
    ql/FiniteDifferences/Makefile
    ql/Indexes/Makefile
    ql/Instruments/Makefile
    ql/Lattices/Makefile
    ql/Math/Makefile
    ql/MonteCarlo/Makefile
    ql/Optimization/Makefile
    ql/Patterns/Makefile
    ql/Pricers/Makefile
    ql/PricingEngines/Makefile
    ql/PricingEngines/Asian/Makefile
    ql/PricingEngines/Barrier/Makefile
    ql/PricingEngines/Basket/Makefile
    ql/PricingEngines/CapFloor/Makefile
    ql/PricingEngines/Cliquet/Makefile
    ql/PricingEngines/Forward/Makefile
    ql/PricingEngines/Quanto/Makefile
    ql/PricingEngines/Swaption/Makefile
    ql/PricingEngines/Vanilla/Makefile
    ql/Processes/Makefile
    ql/RandomNumbers/Makefile
    ql/ShortRateModels/Makefile
    ql/ShortRateModels/CalibrationHelpers/Makefile
    ql/ShortRateModels/OneFactorModels/Makefile
    ql/ShortRateModels/TwoFactorModels/Makefile
    ql/Solvers1D/Makefile
    ql/TermStructures/Makefile
    ql/Utilities/Makefile
    ql/Volatilities/Makefile
    functions/Makefile
    functions/ql/Makefile
    functions/ql/Functions/Makefile
    man/Makefile
    Docs/Makefile
    Examples/Makefile
    Examples/AmericanOption/Makefile
    Examples/BermudanSwaption/Makefile
    Examples/DiscreteHedging/Makefile
    Examples/EuropeanOption/Makefile
    Examples/Swap/Makefile
    test-suite/Makefile])
AC_CONFIG_FILES([quantlib-config],[chmod +x quantlib-config])
AC_CONFIG_FILES([QuantLib.spec])
AC_OUTPUT
