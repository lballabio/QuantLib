
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

/*! \file qlerrors.h
    \brief Classes and functions for error handling.

    $Source$
    $Name$
    $Log$
    Revision 1.12  2001/02/26 18:22:36  lballabio
    Replaced std::domain_error with QuantLib::IndexError

    Revision 1.11  2001/02/26 16:59:01  lballabio
    Wrapped license line

    Revision 1.10  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.9  2001/01/04 20:06:27  nando
    no real change

    Revision 1.8  2000/12/28 11:40:20  aleppo
    Modified QL_REQUIRE and others

    Revision 1.7  2000/12/27 17:18:35  lballabio
    Changes for compiling under Linux and Alpha Linux

    Revision 1.6  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.5  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_error_h
#define quantlib_error_h

#include "qldefines.h"
#include <exception>
#include <string>

namespace QuantLib {

    //! Base error class
    class Error : public std::exception {
      public:
        explicit Error(const std::string& what = "") : message(what) {}
        //! returns the error message.
        const char* what() const throw () { return message.c_str(); }
      private:
        std::string message;
    };

    //! Specialized error
    /*! Thrown upon a failed assertion.
    */
    class AssertionFailedError : public Error {
      public:
        explicit AssertionFailedError(const std::string& what = "")
        : Error(what) {}
    };

    //! Specialized error
    /*! Thrown upon an unsatisfied precondition.
    */
    class PreconditionNotSatisfiedError : public Error {
      public:
        explicit PreconditionNotSatisfiedError(const std::string& what = "")
        : Error(what) {}
    };

    //! Specialized error
    /*! Thrown upon an unsatisfied postcondition.
    */
    class PostconditionNotSatisfiedError : public Error {
      public:
        explicit PostconditionNotSatisfiedError(const std::string& what = "")
        : Error(what) {}
    };

    //! Specialized error
    /*! Thrown upon accessing an array or container
        outside its range.
    */
    class IndexError : public Error {
      public:
        explicit IndexError(const std::string& what = "")
        : Error(what) {}
    };

    //! Specialized error
    /*! Thrown upon passing an argument with an illegal value.
    */
    class IllegalArgumentError : public Error {
      public:
        explicit IllegalArgumentError(const std::string& what = "")
        : Error(what) {}
    };

    //! Specialized error
    /*! Thrown upon obtaining a result outside the allowed range.
    */
    class IllegalResultError : public Error {
      public:
        explicit IllegalResultError(const std::string& what = "")
        : Error(what) {}
    };

    //! Specialized error
    /*! Thrown upon failed allocation.
    */
    class OutOfMemoryError : public Error {
      public:
        explicit OutOfMemoryError(const std::string& whatClass = "unknown class")
        : Error(whatClass+": out of memory") {}
    };

}

/*! \def QL_ASSERT
    \brief it throws an error if the given condition is not verified
    \relates Error
*/
#define QL_ASSERT(condition,description) \
    (condition) || (throw QuantLib::AssertionFailedError(description),false)


/*! \def QL_REQUIRE
    \brief it throws an error if the given pre-condition is not verified
    \relates Error
*/
#define QL_REQUIRE(condition,description) \
    (condition) || \
    (throw QuantLib::PreconditionNotSatisfiedError(description),false)


/*! \def QL_ENSURE
    \brief it throws an error if the given post-condition is not verified
    \relates Error
*/
#define QL_ENSURE(condition,description) \
    (condition) || \
    (throw QuantLib::PostconditionNotSatisfiedError(description),false)



#endif

