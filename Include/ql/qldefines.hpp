
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file qldefines.hpp
    \fullpath Include/ql/%qldefines.hpp
    \brief Global definitions and compiler switches.

*/

// $Id$
// $Log$
// Revision 1.14  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.13  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.12  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/07/24 16:59:34  nando
// documentation revised
//
// Revision 1.9  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.8  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.7  2001/06/05 12:45:27  nando
// R019-branch-merge4 merged into trunk
//
// Revision 1.6  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.5  2001/05/31 14:48:10  lballabio
// Worked around Visual C++ deficiencies
//

/*! \namespace QuantLib
    \brief a.k.a. the %QuantLib Foundation

    The root namespace <tt>%QuantLib</tt> contains what can be considered
    the <b>%QuantLib foundation</b>, i.e., the core of abstract classes upon
    which the rest of the library is built.
*/

#ifndef quantlib_defines_h
#define quantlib_defines_h

/*! \defgroup macros Global QuantLib macros
    Global definitions and quite a few macros which help porting the code to
    different compilers
    @{
*/

//! global trace level (may be superseded locally by a greater value)
#define QL_TRACE_LEVEL 0

#if   defined(__BORLANDC__)     // Borland C++ 5.5
    #include "ql/config.bcc.hpp"
#elif defined(__MWERKS__)       // Metrowerks CodeWarrior
    #include "ql/config.mwcw.hpp"
#elif defined(_MSC_VER)         // Microsoft Visual C++ 6.0
    #include "ql/config.msvc.hpp"
#elif defined(HAVE_CONFIG_H)    // Dynamically created by configure
    #include "ql/config.hpp"
#else
    #error Unsupported compiler - please contact the QuantLib team
#endif


/*! \def QL_DUMMY_RETURN
    \brief Is a dummy return statement required?

    Some compilers will issue a warning if it is missing even though
    it could never be reached during execution, e.g., after a block like
    \code
    if (condition)
        return validResult;
    else
        throw HideousError();
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
    Defines the value of the maximum representable negative integer value
*/
/*! \def QL_MAX_INT
    Defines the value of the maximum representable integer value
*/
/*! \def QL_MIN_DOUBLE
    Defines the value of the maximum representable negative double value
*/
/*! \def QL_MAX_DOUBLE
    Defines the value of the maximum representable double value
*/
/*! \def QL_EPSILON
    Defines the machine precision for operations over doubles
*/
#if defined HAVE_LIMITS
    #include <limits>
    #define QL_MIN_INT      std::numeric_limits<int>::min()
    #define QL_MAX_INT      std::numeric_limits<int>::max()
    #define QL_MIN_DOUBLE  -std::numeric_limits<double>::max()
    #define QL_MAX_DOUBLE   std::numeric_limits<double>::max()
    #define QL_EPSILON      std::numeric_limits<double>::epsilon()
    #define QL_MIN_POSITIVE_DOUBLE  std::numeric_limits<double>::min()
#elif defined HAVE_FLOAT_H
    #if defined HAVE_CLIMITS
        #include <climits>
        #include <float.h>
        #define QL_MIN_INT              INT_MIN
        #define QL_MAX_INT              INT_MAX
        #define QL_MIN_DOUBLE          -DBL_MAX
        #define QL_MAX_DOUBLE           DBL_MAX
        #define QL_EPSILON              DBL_EPSILON
        #define QL_MIN_POSITIVE_DOUBLE  DBL_EPSILON
    #elif defined HAVE_LIMITS_H
        #include <limits.h>
        #include <float.h>
        #define QL_MIN_INT              INT_MIN
        #define QL_MAX_INT              INT_MAX
        #define QL_MIN_DOUBLE          -DBL_MAX
        #define QL_MAX_DOUBLE           DBL_MAX
        #define QL_EPSILON              DBL_EPSILON
        #define QL_MIN_POSITIVE_DOUBLE  DBL_EPSILON
    #endif
#else
    #error Neither <limits>, <climits> nor <limits.h> found
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
#if defined HAVE_CTIME
    #include <ctime>
#elif defined HAVE_TIME_H
    #include <time.h>
#else
    #error Neither <ctime> nor <time.h> found
#endif
/*! @} */


/*! \defgroup charMacros Character functions
    Some compilers still define character functions in the global namespace.
    For the code to be portable these macros should be used instead of
    the actual functions.
    @{
*/
/*! \def QL_STRLEN  \brief string length */
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
    template <class T> T __quantlib_min(const T& x, const T& y) {
        return x < y ? x : y;
    }
    #define QL_MIN  __quantlib_min
#endif
#if !defined(QL_MAX)
    template <class T> T __quantlib_max(const T& x, const T& y) {
        return x < y ? x : y;
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

/*! \def QL_EXPRESSION_TEMPLATES_WORK
    Expression templates techniques (see T. L. Veldhuizen, <i>Expression
    templates</i>, C++ Report, 7(5):26-31, June 1995, available at
    http://extreme.indiana.edu/~tveldhui/papers) are sometimes too advanced for
    the template implementation of current compilers.
*/
#if defined(HAVE_EXPRESSION_TEMPLATES)
    #define QL_EXPRESSION_TEMPLATES_WORK    1
#else
    // Play it safe
    #define QL_EXPRESSION_TEMPLATES_WORK    0
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
    When using the %QuantLib implementation of iterator_traits, this macro might
    be needed to specialize QL_ITERATOR_TRAITS for a pointer to a user-defined
    type.
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
#else
    #define QL_SPECIALIZE_ITERATOR_TRAITS(T)
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

/*! \def QL_ITERATOR
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

/*! @}  */


#endif
