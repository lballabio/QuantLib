
/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file calendars.hpp
    \brief %calendar class non-member non-friend functions
*/

#ifndef quantlib_functions_calendar_h
#define quantlib_functions_calendar_h

#include <ql/Functions/qlfunctions.hpp>
#include <ql/calendar.hpp>
#include <vector>

namespace QuantLib {

    //! Returns the holidays between two dates
    std::vector<Date> holidayList(const Calendar& calendar,
                                  const Date& from,
                                  const Date& to,
                                  bool includeWeekEnds = false);

}


#endif
