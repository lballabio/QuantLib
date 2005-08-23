/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file qldefines.hpp
    \brief Global definitions and compiler switches.
*/

#ifndef quantlib_defines_hpp
/* install-hook */
#define quantlib_defines_hpp

#include <boost/config.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 103100
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

//! version string
#ifdef QL_DEBUG
    #define QL_VERSION "0.3.11-debug"
#else
    #define QL_VERSION "0.3.11"
#endif

//! version hexadecimal number
#define QL_HEX_VERSION 0x000311f0
//! version string for output lib name
#define QL_LIB_VERSION "0_3_11"

#if   defined(HAVE_CONFIG_H)    // Dynamically created by configure
    #include <ql/config.hpp>
#elif defined(__BORLANDC__)     // Borland C++ 5.5
    #include <ql/config.bcc.hpp>
#elif defined(__MWERKS__)       // Metrowerks CodeWarrior
    #include <ql/config.mwcw.hpp>
/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#elif defined(BOOST_MSVC)       // Microsoft Visual C++
    #include <ql/config.msvc.hpp>
#elif defined(__MINGW32__)      // Minimalistic GNU for Windows
    #include <ql/config.mingw.hpp>
#else                           // We hope that the compiler follows ANSI
    #include <ql/config.ansi.hpp>
#endif


/*! \defgroup miscMacros Generic macros

    Miscellaneous macros for compiler idiosyncrasies not fitting other
    categories.

    @{
*/

/*! \def QL_DUMMY_RETURN
    \brief Is a dummy return statement required?

    Some compilers will issue a warning if it is missing even though
    it could never be reached during execution, e.g., after a block like
    \code
    if (condition)
        return validResult;
    else
        QL_FAIL("whatever the reason");
    \endcode
    On the other hand, other compilers will issue a warning if it is present
    because it cannot be reached.
    For the code to be portable this macro should be used after the block.
*/
#if defined(REQUIRES_DUMMY_RETURN)
    #define QL_DUMMY_RETURN(x)        return x;
#else
    #define QL_DUMMY_RETURN(x)
#endif

/*! \def QL_IO_INIT
    \brief I/O initialization

    Sometimes, programs compiled with the free Borland compiler will
    crash miserably upon attempting to write on std::cout.  Strangely
    enough, issuing the instruction
    \code
    std::cout << std::string();
    \endcode
    at the beginning of the program will prevent other accesses to
    <code>std::cout</code> from crashing the program. This macro, to
    be called at the beginning of <code>main()</code>, encapsulates
    the above enchantment for Borland and is defined as empty for the
    other compilers.
*/
#if defined(QL_PATCH_BORLAND)
    #define QL_IO_INIT    std::cout << std::string();
#else
    #define QL_IO_INIT
#endif
/*! @} */


#include <cmath>
#if defined(BOOST_NO_STDC_NAMESPACE)
    namespace std { using ::sqrt; using ::fabs; using ::exp; using ::log;
                    using ::sin; using ::cos; using ::pow; using ::modf;
                    using ::sinh; using ::cosh; using ::floor; using ::fmod; }
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


/*! \defgroup templateMacros Template capabilities

    Some compilers still do not fully implement the template syntax.
    These macros can be used to select between alternate
    implementations of blocks of code, namely, one that takes
    advantage of template programming techniques and a less efficient
    one which is compatible with all compilers.

    @{
*/

/*! \def QL_TYPENAME

    In Visual C++ 6,  typename can only be used in template declarations
    and not in template definitions.
*/
#if defined(CHOKES_ON_TYPENAME)
    #define QL_TYPENAME
#else
    #define QL_TYPENAME typename
#endif
/*! @} */


/*! \defgroup iteratorMacros Iterator support

    Some compilers still define the iterator struct outside the std
    namespace, only partially implement it, or do not implement it at
    all.  For the code to be portable these macros should be used
    instead of the actual functions.

    @{
*/
/*! \def QL_FULL_ITERATOR_SUPPORT
    Some compilers (most notably, Visual C++ 6) still do not fully
    support iterators in their STL implementation.  This macro can be
    used to select between alternate implementations of blocks of
    code, namely, one that takes advantage of full iterator support
    and a less efficient one which is compatible with all compilers.
*/
#if !defined(HAVE_INCOMPLETE_ITERATOR_SUPPORT)
    #define QL_FULL_ITERATOR_SUPPORT
#endif

/*! @}  */

/*! @}  */

#endif
