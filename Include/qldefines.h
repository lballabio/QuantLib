
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file qldefines.h
    \brief Global definitions and compiler switches.

    This file contains a few global definitions and quite a few macros
    which help porting the code to different compilers (each with its own
    non-standard behaviors and implementations).

    $Source$
    $Name$
    $Log$
    Revision 1.23  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.22  2001/01/09 18:31:18  enri
    gnu autotools files added. QuantLib autoconfiscation in progress....

    Revision 1.21  2001/01/09 11:56:44  lballabio
    Added QL_ITERATOR macro

    Revision 1.20  2000/12/27 17:18:35  lballabio
    Changes for compiling under Linux and Alpha Linux

    Revision 1.19  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.18  2000/12/20 15:27:02  lballabio
    Added new defines for helping Linux port

    Revision 1.17  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

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

#ifndef HAVE_CONFIG_H

//! global trace level (may be superseded locally by a greater value)
#define QL_TRACE_LEVEL 0

// Compiler-dependent switches

#if defined(_MSC_VER)                    // Microsoft Visual C++ 6.0
    // disable useless warnings
    #pragma warning(disable: 4786)  // identifier truncated in debug info
    // set switches
    #define QL_USE_NEW_HEADERS                    1
    #define QL_CMATH_IN_STD                        0
    #define QL_CCTYPE_IN_STD                    0
    #define QL_CTIME_IN_STD                        0
    #define QL_ITERATOR_IN_STD                  1
    #define QL_HAS_LIMITS                        1
    #define QL_HAS_MIN_AND_MAX                    1
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS    1
    #define QL_EXPRESSION_TEMPLATES_WORK        1
    #define QL_REQUIRES_DUMMY_RETURN            1
    #define QL_BROKEN_TEMPLATE_SPECIALIZATION    1
    #define QL_GARBLED_MIN_AND_MAX                1
    #define QL_GARBLED_REVERSE_ITERATORS        1
    #define QL_GARBLED_PTR_CONST                1

#elif defined(__BORLANDC__)                // Borland C++ 5.5
    // set switches
    #define QL_USE_NEW_HEADERS                    1
    #define QL_CMATH_IN_STD                        1
    #define QL_CCTYPE_IN_STD                    1
    #define QL_CTIME_IN_STD                        1
    #define QL_ITERATOR_IN_STD                  1
    #define QL_HAS_LIMITS                        1
    #define QL_HAS_MIN_AND_MAX                    1
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS    0
    #define QL_EXPRESSION_TEMPLATES_WORK        1
    #define QL_REQUIRES_DUMMY_RETURN            0
    #define QL_BROKEN_TEMPLATE_SPECIALIZATION    0
    #define QL_GARBLED_MIN_AND_MAX                0
    #define QL_GARBLED_REVERSE_ITERATORS        0
    #define QL_GARBLED_PTR_CONST                0

#elif defined(__MWERKS__)                // Metrowerks CodeWarrior 4.0
    // set switches
    #define QL_USE_NEW_HEADERS                    1
    #define QL_CMATH_IN_STD                        1
    #define QL_CCTYPE_IN_STD                    1
    #define QL_CTIME_IN_STD                        1
    #define QL_ITERATOR_IN_STD                  1
    #define QL_HAS_LIMITS                        1
    #define QL_HAS_MIN_AND_MAX                    1
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS    1
    #define QL_EXPRESSION_TEMPLATES_WORK        1
    #define QL_REQUIRES_DUMMY_RETURN            1
    #define QL_BROKEN_TEMPLATE_SPECIALIZATION    0
    #define QL_GARBLED_MIN_AND_MAX                0
    #define QL_GARBLED_REVERSE_ITERATORS        0
    #define QL_GARBLED_PTR_CONST                0

#elif defined(__GNUC__)                    // GNU C++ 2.95.2
    // set switches
    #define QL_USE_NEW_HEADERS                    1
    #define QL_CMATH_IN_STD                        0
    #define QL_CCTYPE_IN_STD                    0
    #define QL_CTIME_IN_STD                        0
    #define QL_ITERATOR_IN_STD                  0
    #define QL_HAS_LIMITS                        0
    #define QL_HAS_MIN_AND_MAX                    1
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS    0
    #define QL_EXPRESSION_TEMPLATES_WORK        0
    #define QL_REQUIRES_DUMMY_RETURN            0
    #define QL_BROKEN_TEMPLATE_SPECIALIZATION    0
    #define QL_GARBLED_MIN_AND_MAX                0
    #define QL_GARBLED_REVERSE_ITERATORS        0
    #define QL_GARBLED_PTR_CONST                0

#elif defined(__DECCXX)                    // Compaq Alpha C++ 6.3
    // set switches
    #define QL_USE_NEW_HEADERS                    0
    #define QL_CMATH_IN_STD                        0
    #define QL_CCTYPE_IN_STD                    0
    #define QL_CTIME_IN_STD                        0
    #define QL_ITERATOR_IN_STD                  1
    #define QL_HAS_LIMITS                        1
    #define QL_HAS_MIN_AND_MAX                    0
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS    0
    #define QL_EXPRESSION_TEMPLATES_WORK        0
    #define QL_REQUIRES_DUMMY_RETURN            0
    #define QL_BROKEN_TEMPLATE_SPECIALIZATION    0
    #define QL_GARBLED_MIN_AND_MAX                0
    #define QL_GARBLED_REVERSE_ITERATORS        0
    #define QL_GARBLED_PTR_CONST                0

#else                                    // Generic ANSI C++ compliant compiler
    // set switches
    #define QL_USE_NEW_HEADERS                    1
    #define QL_CMATH_IN_STD                        1
    #define QL_CCTYPE_IN_STD                    1
    #define QL_CTIME_IN_STD                        1
    #define QL_ITERATOR_IN_STD                  1
    #define QL_HAS_LIMITS                        1
    #define QL_HAS_MIN_AND_MAX                    1
    #define QL_TEMPLATE_METAPROGRAMMING_WORKS    1
    #define QL_EXPRESSION_TEMPLATES_WORK        1
    #define QL_REQUIRES_DUMMY_RETURN            0
    #define QL_BROKEN_TEMPLATE_SPECIALIZATION    0
    #define QL_GARBLED_MIN_AND_MAX                0
    #define QL_GARBLED_REVERSE_ITERATORS        0
    #define QL_GARBLED_PTR_CONST                0

#endif


// Switch-dependent #definitions

/*! \def QL_EXPRESSION_TEMPLATES_WORK
    Expression templates techniques (see T. L. Veldhuizen, <i>Expression
    templates</i>, C++ Report, 7(5):26-31, June 1995, available at
    http://extreme.indiana.edu/~tveldhui/papers) are sometimes too advanced for
    the template implementation of current compilers.
*/

/*! \def QL_TEMPLATE_METAPROGRAMMING_WORKS
    Template metaprogramming techniques (see T. L. Veldhuizen, <i>Using C++
    Template Metaprograms</i>, C++ Report, Vol 7 No. 4, May 1995, available at
    http://extreme.indiana.edu/~tveldhui/papers) are sometimes too advanced for
    the template implementation of current compilers.
*/

/*! \def QL_USE_NEW_HEADERS
    \brief Are C headers included as, say, <cmath>, or still as <math.h>?

    Some compilers still do not implement the new standard.
*/

/*! \def QL_CMATH_IN_STD
    \brief Are math functions defined in the std namespace?

    Some compilers still define them in the global namespace.
    This switch affects the definition of the macros QL_SQRT, QL_FABS, QL_EXP,
    QL_LOG, QL_SIN, QL_COS, QL_POW, and QL_MODF which should be used instead of
    the actual functions.
*/

#if QL_USE_NEW_HEADERS
    #include <cmath>
#else
    #include <math.h>
#endif

#if QL_CMATH_IN_STD
    /*! \def QL_SQRT \see QL_CMATH_IN_STD */
    #define QL_SQRT    std::sqrt
    /*! \def QL_FABS \see QL_CMATH_IN_STD */
    #define QL_FABS    std::fabs
    /*! \def QL_EXP \see QL_CMATH_IN_STD */
    #define QL_EXP    std::exp
    /*! \def QL_LOG \see QL_CMATH_IN_STD */
    #define QL_LOG    std::log
    /*! \def QL_SIN \see QL_CMATH_IN_STD */
    #define QL_SIN    std::sin
    /*! \def QL_COS \see QL_CMATH_IN_STD */
    #define QL_COS    std::cos
    /*! \def QL_POW \see QL_CMATH_IN_STD */
    #define QL_POW    std::pow
    /*! \def QL_MODF \see QL_CMATH_IN_STD */
    #define QL_MODF    std::modf
#else
    #define QL_SQRT    sqrt
    #define QL_FABS    fabs
    #define QL_EXP    exp
    #define QL_LOG    log
    #define QL_SIN    sin
    #define QL_COS    cos
    #define QL_POW    pow
    #define QL_MODF    modf
#endif

/*! \def QL_CCTYPE_IN_STD
    \brief Are string functions defined in the std namespace?

    Some compilers still define them in the global namespace.
    This switch affects the definition of the macros QL_STRLEN, QL_TOLOWER, and
    QL_TOUPPER which should be used instead of the actual functions.
*/

#if QL_USE_NEW_HEADERS
    #include <cctype>
#else
    #include <ctype.h>
#endif

#if QL_CCTYPE_IN_STD
    /*! \def QL_STRLEN \see QL_CCTYPE_IN_STD */
    #define QL_STRLEN    std::strlen
    /*! \def QL_TOLOWER \see QL_CCTYPE_IN_STD */
    #define QL_TOLOWER    std::tolower
    /*! \def QL_TOUPPER \see QL_CCTYPE_IN_STD */
    #define QL_TOUPPER    std::toupper
#else
    #define QL_STRLEN    strlen
    #define QL_TOLOWER    tolower
    #define QL_TOUPPER    toupper
#endif

/*! \def QL_ITERATOR_IN_STD
    \brief Is iterator defined in the std namespace?

    Some compilers still define it in the global namespace.
    This switch affects the definition of the macro QL_ITERATOR which should be
    used instead of the actual class.
*/

#include <iterator>

#if QL_ITERATOR_IN_STD
    /*! \def QL_ITERATOR \see QL_ITERATOR_IN_STD */
    #define QL_ITERATOR    std::iterator
#else
    #define QL_ITERATOR    iterator
#endif

/*! \def QL_CTIME_IN_STD
    \brief Are time functions defined in the std namespace?

    Some compilers still define them in the global namespace.
    This switch affects the definition of the macro QL_CLOCK which should be
    used instead of the actual function.
*/

#if QL_USE_NEW_HEADERS
    #include <ctime>
#else
    #include <time.h>
#endif

#if QL_CTIME_IN_STD
    /*! \def QL_CLOCK \see QL_CTIME_IN_STD */
    #define QL_CLOCK    std::clock
#else
    #define QL_CLOCK    clock
#endif

/*! \def QL_HAS_LIMITS
    \brief Does the <limits> header exist?

    Some compilers do not give an implementation of it yet.
    This switch affects the definition of the macros QL_MIN_INT, QL_MAX_INT,
    QL_MIN_DOUBLE, QL_MAX_DOUBLE, and QL_EPSILON which should be used instead of
    the corresponding method or macro.
*/
#if QL_HAS_LIMITS
    #include <limits>
    /*! \def QL_MIN_INT \see QL_HAS_LIMITS */
    #define QL_MIN_INT        std::numeric_limits<int>::min()
    /*! \def QL_MAX_INT \see QL_HAS_LIMITS */
    #define QL_MAX_INT        std::numeric_limits<int>::max()
    /*! \def QL_MIN_DOUBLE \see QL_HAS_LIMITS */
    #define QL_MIN_DOUBLE    std::numeric_limits<double>::min()
    /*! \def QL_MAX_DOUBLE \see QL_HAS_LIMITS */
    #define QL_MAX_DOUBLE    std::numeric_limits<double>::max()
    /*! \def QL_EPSILON \see QL_HAS_LIMITS */
    #define QL_EPSILON        std::numeric_limits<double>::epsilon()
#else
    #include <limits.h>
    #include <float.h>
    #define QL_MIN_INT        INT_MIN
    #define QL_MAX_INT        INT_MAX
    #define QL_MIN_DOUBLE    DBL_MIN
    #define QL_MAX_DOUBLE    DBL_MAX
    #define QL_EPSILON        DBL_EPSILON
#endif

/*! \def QL_GARBLED_MIN_AND_MAX
    \brief Blame Microsoft for this one...

    They decided to call them <tt>std::_cpp_min</tt> and <tt>std::_cpp_max</tt>
    to avoid the hassle of rewriting their code.
    This switch affects the definition of the macros QL_MIN and QL_MAX which
    should be used instead of the actual functions.
*/

/*! \def QL_HAS_MIN_AND_MAX
    \brief are std::min and std::max defined?

    Some compilers do not implement them yet.
    This switch affects the definition of the macros QL_MIN and QL_MAX which
    should be used instead of the actual functions.
*/

#include <algorithm>
#if QL_GARBLED_MIN_AND_MAX
    #define QL_MIN    std::_cpp_min
    #define QL_MAX    std::_cpp_max
#elif QL_HAS_MIN_MAX
    /*! \def QL_MIN \see QL_HAS_MIN_MAX \see QL_GARBLED_MIN_AND_MAX */
    #define QL_MIN    std::min
    /*! \def QL_MIN \see QL_HAS_MIN_MAX \see QL_GARBLED_MIN_AND_MAX */
    #define QL_MAX    std::max
#else
    template <class T> T QL_MIN(const T& x, const T& y) {
        return x < y ? x : y; }
    template <class T> T QL_MAX(const T& x, const T& y) {
        return x > y ? x : y; }
#endif


#else
#include "config.h"
#if defined HAVE_CMATH
# include <cmath>
#elif defined HAVE_MATH_H
# include <math.h>
#else
# error neither <cmath> nor <math.h> found by configure!
#endif
#if defined HAVE_LIMITS
# include <limits>
# define QL_MIN_INT      std::numeric_limits<int>::min()
# define QL_MAX_INT     std::numeric_limits<int>::max()
# define QL_MIN_DOUBLE     std::numeric_limits<double>::min()
# define QL_MAX_DOUBLE     std::numeric_limits<double>::max()
# define QL_EPSILON     std::numeric_limits<double>::epsilon()
#elif defined HAVE_FLOAT_H
# if defined HAVE_CLIMITS
#  include <climits>
#  include <float.h>
#  define QL_MIN_INT        INT_MIN
#  define QL_MAX_INT        INT_MAX
#  define QL_MIN_DOUBLE            DBL_MIN
#  define QL_MAX_DOUBLE            DBL_MAX
#  define QL_EPSILON        DBL_EPSILON
# elif defined HAVE_LIMITS_H
#  include <limits.h>
#  include <float.h>
#  define QL_MIN_INT        INT_MIN
#  define QL_MAX_INT        INT_MAX
#  define QL_MIN_DOUBLE            DBL_MIN
#  define QL_MAX_DOUBLE            DBL_MAX
#  define QL_EPSILON        DBL_EPSILON
# endif
#else
# error no limits, climits or limits.h!
#endif

#include <algorithm>
#define QL_GARBLED_REVERSE_ITERATORS 0
#define QL_GARBLED_PTR_CONST 0
#define QL_TEMPLATE_METAPROGRAMMING_WORKS    0
#define QL_EXPRESSION_TEMPLATES_WORK        0
#define QL_REQUIRES_DUMMY_RETURN            0
#define QL_BROKEN_TEMPLATE_SPECIALIZATION    0
#define QL_GARBLED_MIN_AND_MAX                0

#endif /* HAVE_CONFIG_H */

/*! \def QL_GARBLED_REVERSE_ITERATORS
    \brief Blame Microsoft for this one...

    They decided that <tt>std::reverse_iterator<iterator></tt> needed an extra
    template argument.
    This switch affects the definition of the macro
    QL_REVERSE_ITERATOR(iterator,type) which should be used instead of the
    actual class.
*/
#include <iterator>
#if QL_GARBLED_REVERSE_ITERATORS
    #define QL_REVERSE_ITERATOR(iterator,type)    \
        std::reverse_iterator< iterator , type >
#else
    /*! \def QL_REVERSE_ITERATOR \see QL_GARBLED_REVERSE_ITERATORS */
    #define QL_REVERSE_ITERATOR(iterator,type) \
        std::reverse_iterator< iterator >
#endif

/*! \def QL_GARBLED_PTR_CONST
    \brief Blame Microsoft for this one...

    They decided to redefine ANSI C++ and use <tt>const *</tt> instead of <tt>*
    const</tt>.
    This switch affects the definition of the macro
    QL_PTR_CONST which should be used instead of the actual type.
*/
#if QL_GARBLED_PTR_CONST
    #define QL_PTR_CONST    const *
#else
    /*! \def QL_PTR_CONST \see QL_GARBLED_PTR_CONST */
    #define QL_PTR_CONST    * const
#endif

/*! \def QL_REQUIRES_DUMMY_RETURN
    \brief Is a dummy return statement required?

    Some compilers will issue a warning if it is missing even though
    it could never be reached during execution, e.g., after a block like
    \code
    if (condition)
        return validResult;
    else
        throw HideousError();
    \endcode
    This switch affects the definition of the macro QL_DUMMY_RETURN(x) which
    should be used after blocks of code like the above.
*/
#if QL_REQUIRES_DUMMY_RETURN
    #define QL_DUMMY_RETURN(x)        return x;
#else
    /*! \def QL_DUMMY_RETURN \see QL_REQUIRES_DUMMY_RETURN */
    #define QL_DUMMY_RETURN(x)
#endif

/*! \def QL_BROKEN_TEMPLATE_SPECIALIZATION
    \brief Blame Microsoft for this one...

    They decided that a declaration and a definition of a specialized template
    function amount to a redefinition and should issue a linker error.
    This switch affects the definition of the macros
    QL_DECLARE_TEMPLATE_SPECIALIZATION(x) and QL_TEMPLATE_SPECIALIZATION which
    should be used instead of
    \code template<> x; \endcode
    and
    \code template<> \endcode,
    respectively.
*/
#if QL_BROKEN_TEMPLATE_SPECIALIZATION
    #define QL_DECLARE_TEMPLATE_SPECIALIZATION(x)
    #define QL_TEMPLATE_SPECIALIZATION
#else
    /*! \def QL_DECLARE_TEMPLATE_SPECIALIZATION
        \see QL_BROKEN_TEMPLATE_SPECIALIZATION */
    #define QL_DECLARE_TEMPLATE_SPECIALIZATION(x)    template<> x;
    /*! \def QL_TEMPLATE_SPECIALIZATION
        \see QL_BROKEN_TEMPLATE_SPECIALIZATION */
    #define QL_TEMPLATE_SPECIALIZATION                template<>
#endif

#endif


