
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

/*! \file errors.hpp
    \brief Classes and functions for error handling.
*/

#ifndef quantlib_errors_hpp
#define quantlib_errors_hpp

#include <ql/qldefines.hpp>
#include <exception>
#include <string>
#include <boost/assert.hpp>

namespace QuantLib {

    //! Base error class
    class Error : public std::exception {
      public:
        /*! The explicit use of this constructor is not advised.
            Use the QL_FAIL macro instead.
        */
        explicit Error(const std::string& what = "") : message_(what) {}
        ~Error() throw() {}
        //! returns the error message.
        const char* what() const throw () { return message_.c_str(); }
        // formats file and line information
        static std::string where(const char* file, long line) {
            #if QL_ERROR_LINES
            char s[100];
            QL_SPRINTF(s,"%s:%ld: ",file,line);
            return std::string(s);
            #else
            return std::string();
            #endif
        }
      private:
        std::string message_;
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

/*! \def QL_FAIL
    \brief throw an error (possibly with file and line information)
*/
#define QL_FAIL(message) \
throw QuantLib::Error(\
    QuantLib::Error::where(__FILE__,__LINE__) +  message)


/*! \def QL_ASSERT
    \brief throw an error if the given condition is not verified
*/
#define QL_ASSERT(condition,description) \
if (!(condition)) \
    throw QuantLib::AssertionFailedError(\
        QuantLib::Error::where(__FILE__,__LINE__) +  description); \
else


/*! \def QL_REQUIRE
    \brief throw an error if the given pre-condition is not verified
*/
#define QL_REQUIRE(condition,description) \
if (!(condition)) \
    throw QuantLib::PreconditionNotSatisfiedError(\
        QuantLib::Error::where(__FILE__,__LINE__) +  description); \
else


/*! \def QL_ENSURE
    \brief throw an error if the given post-condition is not verified
*/
#define QL_ENSURE(condition,description) \
if (!(condition)) \
    throw QuantLib::PostconditionNotSatisfiedError(\
        QuantLib::Error::where(__FILE__,__LINE__) +  description); \
else


#endif

