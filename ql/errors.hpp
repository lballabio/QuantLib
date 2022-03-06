/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

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
#include <ql/shared_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/current_function.hpp>
#include <exception>
#include <sstream>
#include <string>

namespace QuantLib {

    //! Base error class
    class Error : public std::exception {
      public:
        /*! The explicit use of this constructor is not advised.
            Use the QL_FAIL macro instead.
        */
        Error(const std::string& file,
              long line,
              const std::string& functionName,
              const std::string& message = "");
        #ifdef QL_PATCH_MSVC_2013
        /*! the automatically generated destructor would
            not have the throw specifier.
        */
        ~Error() throw() override {}
        #endif
        //! returns the error message.
        const char* what() const QL_NOEXCEPT override;

      private:
        ext::shared_ptr<std::string> message_;
    };

}

#define QL_MULTILINE_FAILURE_BEGIN do {

/* Disable warning C4127 (conditional expression is constant) when
   wrapping macros with the do { ... } while(false) construct on MSVC
*/
#if defined(BOOST_MSVC)
    #define QL_MULTILINE_FAILURE_END \
        __pragma(warning(push)) \
        __pragma(warning(disable:4127)) \
        } while(false) \
        __pragma(warning(pop))
#else
    #define QL_MULTILINE_FAILURE_END } while(false)
#endif


/* on MSVC++13 the do { ... } while(false) construct fails inside a
   for loop without brackets, so we use a dangling else instead */
#if defined(QL_PATCH_MSVC_2013)
#define QL_MULTILINE_ASSERTION_BEGIN
#else
#define QL_MULTILINE_ASSERTION_BEGIN do {
#endif

/* Disable warning C4127 (conditional expression is constant) when
   wrapping macros with the do { ... } while(false) construct on MSVC
*/
#if defined(BOOST_MSVC)
    #if defined(QL_PATCH_MSVC_2013)
    #define QL_MULTILINE_ASSERTION_END else
    #else
    #define QL_MULTILINE_ASSERTION_END \
        __pragma(warning(push)) \
        __pragma(warning(disable:4127)) \
        } while(false) \
        __pragma(warning(pop))
    #endif
#else
    #define QL_MULTILINE_ASSERTION_END } while(false)
#endif


/*! \def QL_FAIL
    \brief throw an error (possibly with file and line information)
*/
#define QL_FAIL(message) \
QL_MULTILINE_FAILURE_BEGIN \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
QL_MULTILINE_FAILURE_END


/*! \def QL_ASSERT
    \brief throw an error if the given condition is not verified
*/
#define QL_ASSERT(condition,message) \
QL_MULTILINE_ASSERTION_BEGIN \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} \
QL_MULTILINE_ASSERTION_END

/*! \def QL_REQUIRE
    \brief throw an error if the given pre-condition is not verified
*/
#define QL_REQUIRE(condition,message) \
QL_MULTILINE_ASSERTION_BEGIN \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} \
QL_MULTILINE_ASSERTION_END

/*! \def QL_ENSURE
    \brief throw an error if the given post-condition is not verified
*/
#define QL_ENSURE(condition,message) \
QL_MULTILINE_ASSERTION_BEGIN \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} \
QL_MULTILINE_ASSERTION_END


#endif



#ifndef id_55bcece915b148130c4b2d3cffe3a976
#define id_55bcece915b148130c4b2d3cffe3a976
inline bool test_55bcece915b148130c4b2d3cffe3a976(const int* i) {
    return i != nullptr;
}
#endif
