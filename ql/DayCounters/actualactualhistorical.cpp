
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

/*! \file actualactualhistorical.cpp
    \brief act/act historical day count convention

    \fullpath
    DayCounters/%actualactualhistorical.cpp
*/

// $Id$

#include "ql/DayCounters/actualactualhistorical.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActualHistorical::yearFraction(
          const Date& d1, const Date& d2, 
          const Date&, const Date&) const {
            QL_REQUIRE(d2>=d1,
                "Invalid reference period");

	        int y1 = d1.year(), y2 = d2.year();
	        double leap1 = (Date::isLeap(y1)?1.:0.),
		        leap2 = (Date::isLeap(y2)?1.:0.);
	        double dib1 = 365 + leap1, dib2 = 365 + leap2;//

	        double sum = y2 - y1 - 1;

	        sum += dayCount(d1, Date(1,(Month)1,y1+1))/dib1;
	        sum += dayCount(Date(1,(Month)1,y2),d2)/dib2;

	        return sum;
        }

    }

}

