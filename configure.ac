# Process this file with autoconf to produce a configure script.
AC_INIT([QuantLib], [0.3.3a1-cvs], 
        [quantlib-dev@lists.sourceforge.net],
        [QuantLib])
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([ql/qldefines.hpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([ql/config.hpp])
AM_INIT_AUTOMAKE([foreign])

AC_COPYRIGHT([Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it under the
terms of the QuantLib license.  You should have received a copy of the
license along with this program; if not, please email ferdinando@ametrano.net
The license is also available online at http://quantlib.org/html/license.html

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.])

AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PATH_PROG([SED], [sed])
AC_PATH_PROG([DOXYGEN], [doxygen])
AC_PATH_PROG([LATEX], [latex])
AC_PATH_PROG([PDFLATEX], [pdflatex])
AC_PATH_PROG([MAKEINDEX], [makeindex])
AC_PATH_PROG([DVIPS], [dvips])

AC_LANG([C++])

QL_CHECK_HEADER([stdlib])
QL_CHECK_TYPE([size_t],[],[$ql_stdlib])
QL_CHECK_FUNC([atoi],["1.0"],[$ql_stdlib])

QL_CHECK_HEADER([stdio])
QL_CHECK_FUNC([sprintf],[s,"foo"],[$ql_stdio])

QL_CHECK_HEADER([math])
QL_CHECK_FUNC([sqrt],[1.0],[$ql_math])
QL_CHECK_FUNC([fabs],[1.0],[$ql_math])
QL_CHECK_FUNC([exp],[1.0],[$ql_math])
QL_CHECK_FUNC([log],[1.0],[$ql_math])
QL_CHECK_FUNC([sin],[1.0],[$ql_math])
QL_CHECK_FUNC([cos],[1.0],[$ql_math])
QL_CHECK_FUNC([pow],[1.0,2],[$ql_math])
QL_CHECK_FUNC([modf],[1.0,&x],[$ql_math])
QL_CHECK_FUNC([floor],[1.0],[$ql_math])

QL_CHECK_CONSTANT([M_PI],[double],[$ql_math],
                  [3.141592653589793238462643383280],[pi])
QL_CHECK_CONSTANT([M_SQRT_2],[double],[$ql_math],
                  [0.7071067811865475244008443621048],[1/sqrt(2)])
QL_CHECK_CONSTANT([M_SQRTPI],[double],[$ql_math],
                  [1.77245385090551602792981],[sqrt(pi)])
QL_CHECK_CONSTANT([M_1_SQRTPI],[double],[$ql_math],
                  [0.564189583547756286948],[1/sqrt(pi)])

QL_CHECK_HEADER([string])
QL_CHECK_FUNC([strlen],["foo"],[$ql_string])

QL_CHECK_HEADER([ctype])
QL_CHECK_FUNC([tolower],['c'],[$ql_ctype])
QL_CHECK_FUNC([toupper],['c'],[$ql_ctype])

QL_CHECK_HEADER([time])
QL_CHECK_TYPE([time_t],[],[$ql_time])
QL_CHECK_TYPE([tm],[],[$ql_time])
QL_CHECK_FUNC([clock],[],[$ql_time])
QL_CHECK_FUNC([time],[0],[$ql_time])
QL_CHECK_GMTIME

AC_CHECK_HEADERS([limits],,[AC_MSG_WARN([<limits> not found])])
QL_CHECK_HEADER([limits])
AC_CHECK_HEADERS([float.h],,[AC_MSG_WARN([<float.h> not found])])

AC_CHECK_HEADERS([algorithm],,[AC_MSG_WARN([<algorithm> not found])])
QL_CHECK_FUNC([min],[1,2],[algorithm])
QL_CHECK_FUNC([max],[1,2],[algorithm])

AC_CHECK_HEADERS([iterator],,[AC_MSG_WARN([<iterator> not found])])
QL_CHECK_TEMPLATE_TYPE([iterator],[std::input_iterator_tag,double],
                       [],[iterator])
QL_CHECK_TEMPLATE_TYPE([iterator_traits],[double*],
                       [],[iterator])

QL_CHECK_NAMESPACES
QL_CHECK_STRING

AM_PATH_CPPUNIT(1.8.0)
AM_CONDITIONAL(CPPUNIT_FOUND, test "x${CPPUNIT_LIBS}" != "x")

AC_C_CONST
AC_C_INLINE

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

# site configuration
AC_ARG_ENABLE([error-lines],
              AC_HELP_STRING([--enable-error-lines],
                             [If enabled, file and line information is
                              added to the error messages thrown by the
                              library.]),
              [ql_error_lines=$enableval],
              [ql_error_lines=no])
if test "$ql_error_lines" = "yes" ; then
   AC_DEFINE([QL_ERROR_LINES],[1],
             [Define this if error messages should include file and line
              information.])
fi

AC_ARG_ENABLE([early-payments],
              AC_HELP_STRING([--enable-early-payments],
                             [If enabled (the default), payments are
                              considered to be settled at the beginning
                              of the day. Therefore, payments occurring
                              at today's date are not included in the NPV
                              of an instrument.]),
              [ql_early_payments=$enableval],
              [ql_early_payments=yes])
if test "$ql_early_payments" = "no" ; then
   AC_DEFINE([QL_LATE_PAYMENTS],[1],
             [Define this if payments occurring today should enter the NPV
              of an instrument.])
else
   AC_DEFINE([QL_EARLY_PAYMENTS],[1],
             [Define this if payments occurring today should not enter
              the NPV of an instrument.])
fi

AC_ARG_ENABLE([early-swap-payments],
              AC_HELP_STRING([--enable-early-swap-payments],
                             [Same as --enable-early-payments.
                              If given, it overrides the former for
                              interest rate swaps.]),
              [ql_early_swap_payments=$enableval])
if test "$ql_early_swap_payments" = "no" ; then
   AC_DEFINE([QL_LATE_SWAP_PAYMENTS],[1],
             [Define this if payments occurring today should enter the NPV
              of a swap.])
else
if test "$ql_early_swap_payments" = "yes" ; then
   AC_DEFINE([QL_EARLY_SWAP_PAYMENTS],[1],
             [Define this if payments occurring today should not enter the NPV
              of a swap.])
fi
fi

# done, output the configured files
AC_CONFIG_FILES([
    Makefile 
    config/Makefile 
    ql/Makefile 
    ql/Calendars/Makefile 
    ql/CashFlows/Makefile 
    ql/DayCounters/Makefile 
    ql/FiniteDifferences/Makefile 
    ql/functions/Makefile 
    ql/Indexes/Makefile 
    ql/Instruments/Makefile 
    ql/Lattices/Makefile 
    ql/Math/Makefile 
    ql/MonteCarlo/Makefile 
    ql/Optimization/Makefile 
    ql/Patterns/Makefile 
    ql/Pricers/Makefile 
    ql/PricingEngines/Makefile 
    ql/RandomNumbers/Makefile 
    ql/ShortRateModels/Makefile 
    ql/ShortRateModels/CalibrationHelpers/Makefile 
    ql/ShortRateModels/OneFactorModels/Makefile 
    ql/ShortRateModels/TwoFactorModels/Makefile 
    ql/Solvers1D/Makefile 
    ql/TermStructures/Makefile 
    ql/Utilities/Makefile 
    ql/Volatilities/Makefile 
    man/Makefile 
    lib/Makefile 
    lib/Mac/Makefile 
    lib/Mac/CodeWarrior/Makefile 
    lib/Win32/Makefile 
    lib/Win32/Borland/Makefile 
    lib/Win32/VisualStudio/Makefile 
    Docs/Makefile 
    Docs/pages/Makefile 
    Docs/Examples/Makefile 
    Docs/images/Makefile 
    Examples/Makefile 
    Examples/BermudanSwaption/Makefile 
    Examples/DiscreteHedging/Makefile 
    Examples/EuropeanOption/Makefile 
    Examples/Swap/Makefile
    test-suite/Makefile])
AC_CONFIG_FILES([quantlib-config],[chmod +x quantlib-config])
AC_OUTPUT
