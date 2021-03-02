/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd)
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2009 Mark Joshi
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file dataparsers.hpp
    \brief Classes used to parse data for input
*/

#ifndef quantlib_data_parsers_hpp
#define quantlib_data_parsers_hpp

#include <ql/time/date.hpp>
#include <vector>
#include <string>

namespace QuantLib {

    namespace io {

        /*! \deprecated Use std::stoi instead.
                        Deprecated in version 1.22.
        */
        QL_DEPRECATED
        inline Integer to_integer(const std::string& s) {
            return std::stoi(s);
        }

    }

    class PeriodParser {
      public:
        static Period parse(const std::string& str);
      private:
        static Period parseOnePeriod(const std::string& str);
    };

    class DateParser {
      public:
        //! Parses a string in a used-defined format.
        /*! This method uses the parsing functions from
            Boost.Date_Time and supports the same formats.
        */
        static Date parseFormatted(const std::string& str,
                                   const std::string& fmt);
        static Date parseISO(const std::string& str);
    };

}


#endif
