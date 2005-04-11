/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/errors.hpp>
#include <stdexcept>

namespace {

    std::string trim(const std::string& file) {
        // find last path separator
        std::string::size_type n = file.find_last_of("/\\");
        if (n == std::string::npos)
            // return the whole thing--it's a naked file name anyway
            return file;
        else
            // keep the file name only
            return file.substr(n+1);
    }

    #if defined(_MSC_VER) || defined(__BORLANDC__)
    // allow Visual Studio integration
    std::string format(
                       #ifdef QL_ERROR_LINES
                       const std::string& file, long line,
                       #else
                       const std::string&, long,
                       #endif
                       const std::string& function,
                       const std::string& message) {
        std::ostringstream msg;
        if (function != "(unknown)")
            msg << function << ": ";
        #ifdef QL_ERROR_LINES
        msg << "\n  " << file << "(" << line << "): \n";
        #endif
        msg << message;
        return msg.str();
    }
    #else
    // use gcc format (e.g. for integration with Emacs)
    std::string format(const std::string& file, long line,
                       const std::string& function,
                       const std::string& message) {
        std::ostringstream msg;
        #if QL_ERROR_LINES
        msg << "\n" << file << ":" << line << ": ";
        #endif
        if (function != "(unknown)")
            msg << "In function `" << function << "': \n";
        msg << message;
        return msg.str();
    }
    #endif

}

namespace boost {

    // must be defined by the user
    void assertion_failed(char const * expr, char const * function,
                          char const * file, long line) {
        throw std::runtime_error(format(file, line, function,
                                        "Boost assertion failed: " +
                                        std::string(expr)));
    }

}

namespace QuantLib {

    Error::Error(const std::string& file, long line,
                 const std::string& function,
                 const std::string& message) {
        message_ = boost::shared_ptr<std::string>(new std::string(
                                      format(file, line, function, message)));
    }

    const char* Error::what() const throw () {
        return message_->c_str();
    }

}

