
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file daycounters.hpp
    \brief day counters functions
*/

#ifndef quantlib_functions_daycounters_h
#define quantlib_functions_daycounters_h

#include <ql/daycounter.hpp>

namespace QuantLib {

    int accrualDays(DayCounter dc, Date d1, Date d2);
    double accrualFactor(DayCounter dc, Date d1, Date d2, Date d3, Date d4);

}


#endif
