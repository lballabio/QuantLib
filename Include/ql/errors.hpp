
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

/*! \file errors.hpp

    \fullpath
    Include/ql/%errors.hpp
    \brief Classes and functions for error handling.

*/

// $Id$
// $Log$
// Revision 1.6  2001/08/31 15:23:44  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.5  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.4  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.3  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.2  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.1  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//

#ifndef quantlib_errors_hpp
#define quantlib_errors_hpp

#include "ql/qldefines.hpp"
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
        explicit OutOfMemoryError(
            const std::string& whatClass = "unknown class")
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

