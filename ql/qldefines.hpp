/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file qldefines.hpp
    \brief Global definitions and compiler switches.
*/

#ifndef quantlib_defines_hpp
/* install-hook */
#define quantlib_defines_hpp

#ifdef _MSC_VER
/* Microsoft-specific, but needs to be defined before
   including <boost/config.hpp> which somehow includes
   <math.h> under VC++10
*/
#define _USE_MATH_DEFINES
#endif

#include <boost/config.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 103900
    #error using an old version of Boost, please update.
#endif
#if !defined(BOOST_ENABLE_ASSERT_HANDLER)
    #define BOOST_ENABLE_ASSERT_HANDLER
#endif

/* eventually these will go into userconfig.hpp.
   For the time being, we hard code them here.
*/
#define QL_INTEGER int
#define QL_BIG_INTEGER long
#define QL_REAL double


/*! \defgroup macros QuantLib macros

    Global definitions and a few macros which help porting the
    code to different compilers.

    @{
*/

#if (defined(_DEBUG) || defined(DEBUG))
    #define QL_DEBUG
#endif

#if   defined(HAVE_CONFIG_H)    // Dynamically created by configure
   #include <ql/config.hpp>
/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#elif defined(BOOST_MSVC)       // Microsoft Visual C++
   #include <ql/config.msvc.hpp>
#elif defined(__MINGW32__)      // Minimalistic GNU for Windows
   #include <ql/config.mingw.hpp>
#elif defined(__SUNPRO_CC)      // Sun Studio
   #include <ql/config.sun.hpp>
#else                           // We hope that the compiler follows ANSI
   #include <ql/config.ansi.hpp>
#endif


// extra debug checks
#ifdef QL_DEBUG
    #ifndef QL_EXTRA_SAFETY_CHECKS
        #define QL_EXTRA_SAFETY_CHECKS
    #endif
#endif


// ensure that needed math constants are defined
#include <ql/mathconstants.hpp>


// import global functions into std namespace
#if defined(BOOST_NO_STDC_NAMESPACE)
    #include <cmath>
    namespace std {
        using ::sqrt; using ::abs; using ::fabs;
        using ::exp; using ::log; using ::pow;
        using ::sin; using ::cos; using ::asin; using ::acos;
        using ::sinh; using ::cosh;
        using ::floor; using ::fmod; using ::modf;
    }
#endif


/*! \defgroup limitMacros Numeric limits

    Some compilers do not give an implementation of
    <code>\<limits\></code> yet.  For the code to be portable
    these macros should be used instead of the corresponding method of
    <code>std::numeric_limits</code> or the corresponding macro
    defined in <code><limits.h></code>.

    @{
*/
/*! \def QL_MIN_INTEGER
    Defines the value of the largest representable negative integer value
*/
/*! \def QL_MAX_INTEGER
    Defines the value of the largest representable integer value
*/
/*! \def QL_MIN_REAL
    Defines the value of the largest representable negative
    floating-point value
*/
/*! \def QL_MIN_POSITIVE_REAL
    Defines the value of the smallest representable positive double value
*/
/*! \def QL_MAX_REAL
    Defines the value of the largest representable floating-point value
*/
/*! \def QL_EPSILON
    Defines the machine precision for operations over doubles
*/
#include <boost/limits.hpp>
// limits used as such
#define QL_MIN_INTEGER         ((std::numeric_limits<QL_INTEGER>::min)())
#define QL_MAX_INTEGER         ((std::numeric_limits<QL_INTEGER>::max)())
#define QL_MIN_REAL           -((std::numeric_limits<QL_REAL>::max)())
#define QL_MAX_REAL            ((std::numeric_limits<QL_REAL>::max)())
#define QL_MIN_POSITIVE_REAL   ((std::numeric_limits<QL_REAL>::min)())
#define QL_EPSILON             ((std::numeric_limits<QL_REAL>::epsilon)())
// specific values---these should fit into any Integer or Real
#define QL_NULL_INTEGER        ((std::numeric_limits<int>::max)())
#define QL_NULL_REAL           ((std::numeric_limits<float>::max)())
/*! @} */

/*! @}  */


// emit warning when using deprecated features
#if defined(BOOST_MSVC)       // Microsoft Visual C++
#define QL_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) || defined(__clang__)
#define QL_DEPRECATED __attribute__((deprecated))
#else
// we don't know how to enable it, just define the macro away
#define QL_DEPRECATED
#endif


#endif
