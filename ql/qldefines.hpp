
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

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

    Global definitions and quite a few macros which help porting the
    code to different compilers.

    @{
*/

#if (defined(_DEBUG) || defined(DEBUG))
    #define QL_DEBUG
#endif

//! version string
#ifdef QL_DEBUG
    #define QL_VERSION "0.3.9-debug"
#else
    #define QL_VERSION "0.3.9"
#endif

//! version hexadecimal number
#define QL_HEX_VERSION 0x000309f0
//! version string for output lib name
#define QL_LIB_VERSION "0_3_9"

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


#if defined HAVE_CSTDLIB
    #include <cstdlib>
#elif defined HAVE_STDLIB_H
    #include <stdlib.h>
#else
    #error Neither <cstdlib> nor <stdlib.h> found
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


/*! \defgroup mathMacros Math functions

    Some compilers still define math functions in the global
    namespace.  For the code to be portable these macros should be
    used instead of the actual functions.

    @{
*/
/*! \def QL_SQRT \brief square root */
/*! \def QL_FABS \brief absolute value */
/*! \def QL_EXP  \brief exponential */
/*! \def QL_LOG  \brief logarithm */
/*! \def QL_SIN  \brief sine */
/*! \def QL_COS  \brief cosine */
/*! \def QL_POW  \brief power */
/*! \def QL_MODF \brief floating-point module */
#if defined HAVE_CMATH
    #include <cmath>
#elif defined HAVE_MATH_H
    #include <math.h>
#else
    #error Neither <cmath> nor <math.h> found
#endif
/*! @} */


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
#if defined HAVE_LIMITS
    #include <limits>
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
#elif defined HAVE_FLOAT_H
    #include <float.h>
    #if defined HAVE_CLIMITS
        #include <climits>
    #elif defined HAVE_LIMITS_H
        #include <limits.h>
    #else
        #error Neither <limits>, <climits> nor <limits.h> found
    #endif
    template <class T> struct quantlib_limits__ {};
    template <> struct quantlib_limits__<int> {
        static int min() { return INT_MIN; }
        static int max() { return INT_MAX; }
    };
    template <> struct quantlib_limits__<long> {
        static long min() { return LONG_MIN; }
        static long max() { return LONG_MAX; }
    };
    #if defined(QL_HAVE_LONG_LONG)
    template <> struct quantlib_limits__<long long> {
        static long long min() {
            #if defined(LLONG_MIN)
                return LLONG_MIN;
            #else
                return LONG_LONG_MIN;
            #endif
        }
        static long long max() {
            #if defined(LLONG_MAX)
                return LLONG_MAX;
            #else
                return LONG_LONG_MAX;
            #endif
        }
    };
    #endif
    template <> struct quantlib_limits__<float> {
        static float min() { return FLT_MIN; }
        static float max() { return FLT_MAX; }
        static float epsilon() { return FLT_EPSILON; }
    };
    template <> struct quantlib_limits__<double> {
        static double min() { return DBL_MIN; }
        static double max() { return DBL_MAX; }
        static double epsilon() { return DBL_EPSILON; }
    };
    template <> struct quantlib_limits__<long double> {
        static long double min() { return LDBL_MIN; }
        static long double max() { return LDBL_MAX; }
        static long double epsilon() { return LDBL_EPSILON; }
    };
    // limits used as such
    #define QL_MIN_INTEGER          ((quantlib_limits__<QL_INTEGER>::min)())
    #define QL_MAX_INTEGER          ((quantlib_limits__<QL_INTEGER>::max)())
    #define QL_MIN_REAL            -((quantlib_limits__<QL_REAL>::max)())
    #define QL_MAX_REAL             ((quantlib_limits__<QL_REAL>::max)())
    #define QL_MIN_POSITIVE_REAL    ((quantlib_limits__<QL_REAL>::min)())
    #define QL_EPSILON              ((quantlib_limits__<QL_REAL>::epsilon)())
    // specific values---these should fit into any Integer or Real
    #define QL_NULL_INTEGER         INT_MAX
    #define QL_NULL_REAL            FLT_MAX
#else
    #error Neither <limits> nor <float.h> found
#endif
/*! @} */


/*! \defgroup timeMacros Time functions

    Some compilers still define time functions in the global namespace.
    For the code to be portable these macros should be used instead of
    the actual functions.

    @{
*/
/*! \def QL_CLOCK
    \brief clock value
*/
/*! \def QL_TIME
    \brief time value
*/
#if defined HAVE_CTIME
    #include <ctime>
#elif defined HAVE_TIME_H
    #include <time.h>
#else
    #error Neither <ctime> nor <time.h> found
#endif
/*! @} */


/*! \defgroup strMacros String functions

    Some compilers still define string functions in the global
    namespace.  For the code to be portable these macros should be
    used instead of the actual functions.

    @{
*/
/*! \def QL_STRLEN  \brief string length */
#if defined HAVE_CSTRING
    #include <cstring>
#elif defined HAVE_STRING_H
    #include <string.h>
#else
    #error Neither <cstring> nor <string.h> found
#endif
/*! @} */


/*! \defgroup charMacros Character functions

    Some compilers still define character functions in the global
    namespace.  For the code to be portable these macros should be
    used instead of the actual functions.

    @{
*/
/*! \def QL_TOUPPER \brief convert to uppercase */
/*! \def QL_TOLOWER \brief convert to lowercase */
#if defined HAVE_CCTYPE
    #include <cctype>
#elif defined HAVE_CTYPE_H
    #include <ctype.h>
#else
    #error Neither <cctype> nor <ctype.h> found
#endif
/*! @} */


/*! \defgroup algoMacros Min and max functions

    Some compilers still do not define <code>std::min</code> and
    <code>std::max</code>. Moreover, Visual C++ defines them but for
    unfathomable reasons garble their names.  For the code to be
    portable these macros should be used instead of the actual
    functions.

    @{
*/
/*! \def QL_MIN \brief minimum between two elements */
/*! \def QL_MAX \brief maximum between two elements */
#include <algorithm>
#if !defined(QL_MIN)
    template <class T> inline const T& quantlib_min__(const T& x, const T& y) {
        return x < y ? x : y;
    }
    #define QL_MIN  quantlib_min__
#endif
#if !defined(QL_MAX)
    template <class T> inline const T& quantlib_max__(const T& x, const T& y) {
        return x > y ? x : y;
    }
    #define QL_MAX  quantlib_max__
#endif
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
    \brief Blame Microsoft for this one...

    They decided that typename can only be used in template declarations
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
    Some compilers (most notably, Visual C++) still do not fully
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
