
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

/*! \namespace QuantLib
    \brief a.k.a. the %QuantLib Foundation

    See sect. \ref coreclasses
*/

#ifndef quantlib_defines_h
/* install-hook */
#define quantlib_defines_h

/*! \defgroup macros Global QuantLib macros
    Global definitions and quite a few macros which help porting the code to
    different compilers
    @{
*/

#if defined(_DEBUG) || defined(DEBUG)
    #define QL_DEBUG
#endif

//! version hexadecimal number
#define QL_HEX_VERSION 0x000305f0

//! version string
#ifdef QL_DEBUG
    #define QL_VERSION "0.3.5-debug"
#else
    #define QL_VERSION "0.3.5"
#endif

//! specific per-compiler definitions
#if   defined(__BORLANDC__)     // Borland C++ 5.5
    #include <ql/config.bcc.hpp>
#elif defined(__MWERKS__)       // Metrowerks CodeWarrior
    #include <ql/config.mwcw.hpp>
#elif defined(_MSC_VER)         // Microsoft Visual C++ 6.0
    #include <ql/config.msvc.hpp>
#elif defined(HAVE_CONFIG_H)    // Dynamically created by configure
    #include <ql/config.hpp>
#else
    #error Unsupported compiler - please contact the QuantLib team
#endif


#if defined(HAVE_BOOST)
    #include <boost/version.hpp>
    #if BOOST_VERSION < 103002
        #error using an old version of Boost, please update.
    #endif
    #if !defined(BOOST_ENABLE_ASSERT_HANDLER)
    #define BOOST_ENABLE_ASSERT_HANDLER
    #endif
#endif

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
    crash miserably upon attempting to write on std::cout. 
    Strangely enough, issuing the instruction
    \code
    std::cout << std::string();
    \endcode
    at the beginning of the program will prevent other accesses to
    std::cout from crashing the program. This macro, to be called at
    the beginning of main(), encapsulates the above enchantment for 
    Borland and is defined as empty for the other compilers.
*/
#if defined(__BORLANDC__)
    #define QL_IO_INIT    std::cout << std::string();
#else
    #define QL_IO_INIT
#endif
/*! @} */


/*! \defgroup mathMacros Math functions
    Some compilers still define math functions them in the global namespace.
    For the code to be portable these macros should be used instead of
    the actual functions.
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
    Some compilers do not give an implementation of <limits> yet.
    For the code to be portable these macros should be used instead of the
    corresponding method of std::numeric_limits or the corresponding macro
    defined in <limits.h>.
    @{
*/
/*! \def QL_MIN_INT
    Defines the value of the largest representable negative integer value
*/
/*! \def QL_MAX_INT
    Defines the value of the largest representable integer value
*/
/*! \def QL_MIN_DOUBLE
    Defines the value of the largest representable negative double value
*/
/*! \def QL_MIN_POSITIVE_DOUBLE
    Defines the value of the smallest representable positive double value
*/
/*! \def QL_MAX_DOUBLE
    Defines the value of the largest representable double value
*/
/*! \def QL_EPSILON
    Defines the machine precision for operations over doubles
*/
#if defined HAVE_LIMITS
    #include <limits>
    #define QL_MIN_INT      ((std::numeric_limits<int>::min)())
    #define QL_MAX_INT      ((std::numeric_limits<int>::max)())
    #define QL_MIN_DOUBLE  -((std::numeric_limits<double>::max)())
    #define QL_MAX_DOUBLE   ((std::numeric_limits<double>::max)())
    #define QL_EPSILON      ((std::numeric_limits<double>::epsilon)())
    #define QL_MIN_POSITIVE_DOUBLE ((std::numeric_limits<double>::min)())
#elif defined HAVE_FLOAT_H
    #include <float.h>
    #if defined HAVE_CLIMITS
        #include <climits>
    #elif defined HAVE_LIMITS_H
        #include <limits.h>
    #else
        #error Neither <limits>, <climits> nor <limits.h> found
    #endif
    #define QL_MIN_INT              INT_MIN
    #define QL_MAX_INT              INT_MAX
    #define QL_MIN_DOUBLE          -DBL_MAX
    #define QL_MAX_DOUBLE           DBL_MAX
    #define QL_EPSILON              DBL_EPSILON
    #define QL_MIN_POSITIVE_DOUBLE  DBL_EPSILON
#else
    #error Neither <limits> nor <float.h> found
#endif
/*! @} */

#if defined(_MSC_VER)         // Microsoft Visual C++ 6.0
    namespace {
        // 4 old-style pricer tests fails if the following line is uncommented
        //unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM);
    }
#endif


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
    Some compilers still define string functions in the global namespace.
    For the code to be portable these macros should be used instead of
    the actual functions.
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
    Some compilers still define character functions in the global namespace.
    For the code to be portable these macros should be used instead of
    the actual functions.
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


/*! \defgroup stdioMacros Input/output functions
    Some compilers still define i/o functions in the global namespace.
    For the code to be portable these macros should be used instead of
    the actual functions.
    @{
*/
/*! \def QL_SPRINTF \brief print to string */
#if defined HAVE_CSTDIO
    #include <cstdio>
#elif defined HAVE_STDIO_H
    #include <stdio.h>
#else
    #error Neither <cstdio> nor <stdio.h> found
#endif
/*! @} */


/*! \defgroup algoMacros Min and max functions
    Some compilers still do not define std::min and std::max. Moreover, Visual
    C++ defines them but for unfathomable reasons garble their names.
    For the code to be portable these macros should be used instead of
    the actual functions.
    @{
*/
/*! \def QL_MIN \brief minimum between two elements */
/*! \def QL_MAX \brief maximum between two elements */
#include <algorithm>
#if !defined(QL_MIN)
    template <class T> inline const T& __quantlib_min(const T& x, const T& y) {
        return x < y ? x : y;
    }
    #define QL_MIN  __quantlib_min
#endif
#if !defined(QL_MAX)
    template <class T> inline const T& __quantlib_max(const T& x, const T& y) {
        return x > y ? x : y;
    }
    #define QL_MAX  __quantlib_max
#endif
/*! @} */


/*! \defgroup templateMacros Template capabilities
    Some compilers still do not fully implement the template syntax.
    These macros can be used to select between alternate implementations of
    blocks of code, namely, one that takes advantage of template programming
    techniques and a less efficient one which is compatible with all compilers.
    @{
*/
/*! \def QL_DECLARE_TEMPLATE_SPECIALIZATIONS
    \brief Blame Microsoft for this one...

    They decided that a declaration and a definition of a specialized template
    function amount to a redefinition and should issue a linker error.
    For the code to be portable, template specializations should be declared (as
    opposed to defined) only if this macro is defined.
*/
#if !defined(BROKEN_TEMPLATE_SPECIALIZATION)
    #define QL_DECLARE_TEMPLATE_SPECIALIZATIONS
#endif

/*! \def QL_ALLOW_TEMPLATE_METHOD_CALLS
    \brief Blame Microsoft for this one...

    Their compiler cannot cope with method calls such as
    \code
    Handle<Type1> h1(whatever);
    h2 = h1.downcast<Type2>();
    \endcode

    For compatibility, a workaround should be implemented (which of course will
    be less solid or more comples - as I said, blame Microsoft...)
*/
#if !defined(BROKEN_TEMPLATE_METHOD_CALLS)
    #define QL_ALLOW_TEMPLATE_METHOD_CALLS   1
#else
    #define QL_ALLOW_TEMPLATE_METHOD_CALLS   0
#endif

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


/*! \def QL_TEMPLATE_METAPROGRAMMING_WORKS
    Template metaprogramming techniques (see T. L. Veldhuizen, <i>Using C++
    Template Metaprograms</i>, C++ Report, Vol 7 No. 4, May 1995, available at
    http://extreme.indiana.edu/~tveldhui/papers) are sometimes too advanced for
    the template implementation of current compilers.
*/
#if defined(HAVE_TEMPLATE_METAPROGRAMMING)
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS   1
#else
    // Play it safe
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS   0
#endif
/*! @} */


/*! \defgroup iteratorMacros Iterator support
    Some compilers still define the iterator struct outside the std namespace,
    only partially implement it, or do not implement it at all.
    For the code to be portable these macros should be used instead of
    the actual functions.
    @{
*/
/*! \def QL_ITERATOR
    Custom iterators should be derived from this struct for the code to be
    portable.
*/
#include <iterator>
#if !defined(QL_ITERATOR)
    template <class Category, class T, class Distance = ptrdiff_t,
              class Pointer = T*, class Reference = T&>
    struct __quantlib_iterator {
        typedef T          value_type;
        typedef Distance   difference_type;
        typedef Pointer    pointer;
        typedef Reference  reference;
        typedef Category   iterator_category;
    };
    #define QL_ITERATOR     __quantlib_iterator
#endif

/*! \def QL_ITERATOR_TRAITS
    For the code to be portable this macro should be used instead of the actual
    struct.
*/
/*! \def QL_SPECIALIZE_ITERATOR_TRAITS
    When using the %QuantLib implementation of iterator_traits or
    Visual C++ .Net, this macro might be needed to specialize
    QL_ITERATOR_TRAITS for a pointer to a user-defined type.
*/
#if !defined(QL_ITERATOR_TRAITS)
    template <class Iterator>
    struct __quantlib_iterator_traits {
        typedef typename Iterator::value_type           value_type;
        typedef typename Iterator::difference_type      difference_type;
        typedef typename Iterator::pointer              pointer;
        typedef typename Iterator::reference            reference;
        typedef typename Iterator::iterator_category    iterator_category;
    };
    #define QL_ITERATOR_TRAITS  __quantlib_iterator_traits
    #define QL_SPECIALIZE_ITERATOR_TRAITS(T) \
    template<> \
    struct QL_ITERATOR_TRAITS<T*> { \
        typedef T           value_type; \
        typedef ptrdiff_t   difference_type; \
        typedef T*          pointer; \
        typedef T&          reference; \
        typedef std::random_access_iterator_tag  iterator_category; \
    }; \
    template<> \
    struct QL_ITERATOR_TRAITS<const T*> { \
        typedef T           value_type; \
        typedef ptrdiff_t   difference_type; \
        typedef const T*          pointer; \
        typedef const T&          reference; \
        typedef std::random_access_iterator_tag  iterator_category; \
    };
#elif defined(SPECIALIZE_MS_ITERATOR_TRAITS)
    #define QL_SPECIALIZE_ITERATOR_TRAITS(T) \
    namespace std { \
        template<> \
        struct iterator_traits<T*> { \
            typedef T           value_type; \
            typedef ptrdiff_t   difference_type; \
            typedef T*          pointer; \
            typedef T&          reference; \
            typedef std::random_access_iterator_tag  iterator_category; \
        }; \
        template<> \
        struct iterator_traits<const T*> { \
            typedef T           value_type; \
            typedef ptrdiff_t   difference_type; \
            typedef const T*          pointer; \
            typedef const T&          reference; \
            typedef std::random_access_iterator_tag  iterator_category; \
        }; \
    }
#else
    #define QL_SPECIALIZE_ITERATOR_TRAITS(T)
#endif
// actual specializations
#if !defined(__DOXYGEN__)
QL_SPECIALIZE_ITERATOR_TRAITS(bool)
QL_SPECIALIZE_ITERATOR_TRAITS(char)
QL_SPECIALIZE_ITERATOR_TRAITS(short)
QL_SPECIALIZE_ITERATOR_TRAITS(int)
QL_SPECIALIZE_ITERATOR_TRAITS(long)
QL_SPECIALIZE_ITERATOR_TRAITS(float)
QL_SPECIALIZE_ITERATOR_TRAITS(double)
QL_SPECIALIZE_ITERATOR_TRAITS(long double)
#endif

/*! \def QL_REVERSE_ITERATOR
    \brief Blame Microsoft for this one...

    They decided that <tt>std::reverse_iterator<iterator></tt> needed an extra
    template argument.
    For the code to be portable this macro should be used instead of the
    actual class.
*/
#if defined(GARBLED_REVERSE_ITERATOR)
    #define QL_REVERSE_ITERATOR(iterator,type)    \
        std::reverse_iterator< iterator , type >
#else
    #define QL_REVERSE_ITERATOR(iterator,type) \
        std::reverse_iterator< iterator >
#endif

/*! \def QL_FULL_ITERATOR_SUPPORT
    Some compilers (most notably, Visual C++) still do not fully support
    iterators in their STL implementation.
    This macro can be used to select between alternate implementations of
    blocks of code, namely, one that takes advantage of full iterator support
    and a less efficient one which is compatible with all compilers.
*/
#if !defined(HAVE_INCOMPLETE_ITERATOR_SUPPORT)
    #define QL_FULL_ITERATOR_SUPPORT
#endif

/*! @}  */


namespace QuantLib {

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Math = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Calendars = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace CashFlows = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace DayCounters = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Indexes = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Instruments = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace PricingEngines = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Solvers1D = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace TermStructures = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace VolTermStructures = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace FiniteDifferences = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Lattices = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace MonteCarlo = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Optimization = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Patterns = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Pricers = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace RandomNumbers = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace ShortRateModels = ::QuantLib;
    //! \deprecated inner namespace aliases will be removed in next release
    namespace CalibrationHelpers = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Utilities = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Volatilities = ::QuantLib;

    //! \deprecated inner namespace aliases will be removed in next release
    namespace Functions = ::QuantLib;


}

#endif
