

/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file daycounters.cpp
    \brief day counters functions

    \fullpath
    ql/functions/%daycounters.cpp
*/

// $Id$

#include <ql/functions/daycounters.hpp>

using QuantLib::DayCounter;

namespace QuantLib {

    namespace Functions {
		int accrualDays(DayCounter dc, Date d1, Date d2) {
			return dc.dayCount(d1, d2);
		}


		double accrualFactor(DayCounter dc, Date d1, Date d2, Date d3, Date d4) {
            return dc.yearFraction(d1, d2, d3, d4);
		}
    }

}
