/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/Indexes/libor.hpp>
#include <ql/Calendars/jointcalendar.hpp>

namespace QuantLib {

    Libor::Libor(const std::string& familyName,
                 Integer n, TimeUnit units, Integer settlementDays,
                 const Currency& currency,
                 const Calendar& localCalendar,
                 const Calendar& currencyCalendar,
                 BusinessDayConvention convention,
                 const DayCounter& dayCounter,
                 const Handle<YieldTermStructure>& h)
    : Xibor(familyName, n, units, settlementDays, currency,
            JointCalendar(localCalendar,currencyCalendar,JoinHolidays),
            convention, dayCounter, h),
      localCalendar_(localCalendar), currencyCalendar_(currencyCalendar) {}

    Date Libor::valueDate(const Date& fixingDate) const {
        Date d = localCalendar_.advance(fixingDate,settlementDays_,Days);
        return calendar().adjust(d);
    }

    Date Libor::maturityDate(const Date& valueDate) const {
        Calendar jointCalendar = calendar();
        if (jointCalendar.isEndOfMonth(valueDate)) {
            Date d = valueDate + n_*units_;
            Date last = Date::endOfMonth(d);
            return jointCalendar.adjust(last,Preceding);
        } else {
            return jointCalendar.advance(valueDate, n_, units_, convention_);
        }
    }

}

