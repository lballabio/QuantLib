
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file qldefines.h
    \brief Global definitions and compiler switches.

    This file and the compiler-specific config.h contain a few global 
    definitions and quite a few macros which help porting the code to different 
    compilers (each with its own non-standard behaviors and implementations).

    $Source$
    $Name$
    $Log$
    Revision 1.31  2001/02/15 17:36:13  lballabio
    Added checks for iterator and iterator_traits

    Revision 1.30  2001/02/13 09:58:23  lballabio
    Some more work on iterators

    Revision 1.29  2001/02/12 19:00:39  lballabio
    Some more work on iterators

    Revision 1.28  2001/02/12 18:34:49  lballabio
    Some work on iterators

    Revision 1.27  2001/02/09 19:24:30  lballabio
    Reorganized to include configuration file on all platforms

    Revision 1.26  2001/02/08 17:20:37  lballabio
    Fixed wrapping

    Revision 1.25  2001/02/05 14:49:11  enri
    added some files to Makefile.am files

    Revision 1.24  2001/01/17 16:33:40  nando
    bug fix.
    It was
    # define QL_MIN_DOUBLE  std::numeric_limits<double>::min()
    now it is
    # define QL_MIN_DOUBLE  -std::numeric_limits<double>::max()

    Revision 1.23  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.22  2001/01/09 18:31:18  enri
    gnu autotools files added. QuantLib autoconfiscation in progress....

*/

/*! \mainpage

    \section copyright Copyright
    Copyright (C) 2000 --- The %QuantLib Group

    \section introduction Introduction
    %QuantLib is a C++ open source library for financial quantitative
    analysts and developers --- http://quantlib.sourceforge.net/

    %QuantLib is free software and you are allowed to use, copy, modify, merge,
    publish, distribute, and/or sell copies of it under the conditions stated
    in the %QuantLib License available at
    http://quantlib.sourceforge.net/LICENSE.TXT

    %QuantLib and its documentation are distributed in the hope that they will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the license for
    more details.

    \section disclaimer Disclaimer
    At this time, this documentation is widely incomplete and must be regarded
    as a work in progress. Eventually, all %QuantLib classes will be documented.
    However, no date is currently set for this goal.
    Join the mailing lists (http://sourceforge.net/mail/?group_id=12740) for the
    latest updates.

    \section download Download
    If you want to download the documentation, source code, modules, etc. go to
    http://sourceforge.net/project/showfiles.php?group_id=12740
*/

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
    #include "config.bcc.h"
#elif defined(__MWERKS__)       // Metrowerks CodeWarrior
    #include "config.mwcw.h"
#elif defined(_MSC_VER)         // Microsoft Visual C++ 6.0
    #include "config.msvc.h"
#elif defined(HAVE_CONFIG_H)    // Dynamically created by configure
    #include "config.h"
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
#elif defined HAVE_FLOAT_H
    #if defined HAVE_CLIMITS
        #include <climits>
        #include <float.h>
        #define QL_MIN_INT        INT_MIN
        #define QL_MAX_INT        INT_MAX
        #define QL_MIN_DOUBLE    -DBL_MAX
        #define QL_MAX_DOUBLE     DBL_MAX
        #define QL_EPSILON        DBL_EPSILON
    #elif defined HAVE_LIMITS_H
        #include <limits.h>
        #include <float.h>
        #define QL_MIN_INT        INT_MIN
        #define QL_MAX_INT        INT_MAX
        #define QL_MIN_DOUBLE    -DBL_MAX
        #define QL_MAX_DOUBLE     DBL_MAX
        #define QL_EPSILON        DBL_EPSILON
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
/*! \def QL_CLOCK \brief clock value */
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
/*! \def QL_TEMPLATE_SPECIALIZATION
    \brief Blame Microsoft for this one...

    They decided that a declaration and a definition of a specialized template
    function amount to a redefinition and should issue a linker error.
    For the code to be portable this macro should be used instead of
    the actual syntax.
*/
#if defined(BROKEN_TEMPLATE_SPECIALIZATION)
    #define QL_TEMPLATE_SPECIALIZATION
#else
    #define QL_TEMPLATE_SPECIALIZATION  template<>
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
    Some compilers still define the iterator struct outside the std namespace or 
    do not implement it at all. 
    For the code to be portable these macros should be used instead of
    the actual functions.
    @{
*/
/*! \def QL_ITERATOR 
    \note custom iterators should be derived from this struct
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
/*! @}  */

/*! @}  */


#endif
