

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file errors.hpp
    \brief Classes and functions for error handling.

    \fullpath
    ql/%errors.hpp
*/

// $Id$

#ifndef quantlib_errors_hpp
#define quantlib_errors_hpp

#include <ql/qldefines.hpp>
#include <exception>
#include <string>

namespace QuantLib {

    //! Base error class
    class Error : public std::exception {
      public:
        explicit Error(const std::string& what = "") : message(what) {}
    ~Error() throw() {}
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
    \brief throw an error if the given condition is not verified
    \relates Error
*/
#define QL_ASSERT(condition,description) \
    do { \
        if (!(condition)) \
            throw QuantLib::AssertionFailedError(description); \
    } while (false)


/*! \def QL_REQUIRE
    \brief throw an error if the given pre-condition is not verified
    \relates Error
*/
#define QL_REQUIRE(condition,description) \
    do { \
        if (!(condition)) \
            throw QuantLib::PreconditionNotSatisfiedError(description); \
    } while (false)


/*! \def QL_ENSURE
    \brief throw an error if the given post-condition is not verified
    \relates Error
*/
#define QL_ENSURE(condition,description) \
    do { \
        if (!(condition)) \
            throw QuantLib::PostconditionNotSatisfiedError(description); \
    } while (false)



#endif

