
/*
 Copyright (C) 2000-2005 StatPro Italia srl

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
#include <boost/assert.hpp>
#include <boost/current_function.hpp>
#include <boost/shared_ptr.hpp>
#include <exception>
#include <sstream>

namespace QuantLib {

    //! Base error class
    class Error : public std::exception {
      public:
        /*! The explicit use of this constructor is not advised.
            Use the QL_FAIL macro instead.
        */
        Error(const std::string& file, long line,
              const std::string& function,
              const std::string& message = "");
        /*! the automatically generated destructor would
            not have the throw specifier.
        */
        ~Error() throw() {}
        //! returns the error message.
        const char* what() const throw ();
      private:
        boost::shared_ptr<std::string> message_;
    };

}

/*! \def QL_FAIL
    \brief throw an error (possibly with file and line information)
*/
#define QL_FAIL(message) \
do { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} while (false)


/*! \def QL_ASSERT
    \brief throw an error if the given condition is not verified
*/
#define QL_ASSERT(condition,message) \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} else


/*! \def QL_REQUIRE
    \brief throw an error if the given pre-condition is not verified
*/
#define QL_REQUIRE(condition,message) \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} else


/*! \def QL_ENSURE
    \brief throw an error if the given post-condition is not verified
*/
#define QL_ENSURE(condition,message) \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
} else


#endif

