
/*
 Copyright (C) 2000, 2001, 2002 Andre Louw

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

/*! \file dataparsers.cpp
    \brief classes used to parse data for input

    \fullpath
    ql/%dataparsers.cpp
*/

// $Id$

#include <ql/dataparsers.hpp>

namespace QuantLib {

    Period PeriodParser::parse(const std::string& str) {
        TimeUnit units = Days;

        QL_REQUIRE(str.length()>1, "Argument needs length of at least 2");
        Size iPos = str.find_first_of("DdWwMmYy");
        if (iPos != str.length()-1)
            throw Error("Unknown units");
        char abbr = QL_TOUPPER(str[iPos]);
        if (abbr == 'D')      units = Days;
        else if (abbr == 'W') units = Weeks;
        else if (abbr == 'M') units = Months;
        else if (abbr == 'Y') units = Years;
        return Period(QL_ATOI(str.c_str()), units);
    }

}
