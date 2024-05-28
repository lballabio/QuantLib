/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    IborIndex::IborIndex(std::string familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         const Currency& currency,
                         const Calendar& fixingCalendar,
                         BusinessDayConvention convention,
                         bool endOfMonth,
                         const DayCounter& dayCounter,
                         Handle<YieldTermStructure> h)
    : InterestRateIndex(std::move(familyName), tenor, settlementDays, currency, fixingCalendar, dayCounter),
      convention_(convention), termStructure_(std::move(h)), endOfMonth_(endOfMonth) {
        registerWith(termStructure_);
    }

    Rate IborIndex::forecastFixing(const Date& fixingDate) const {
        Date d1 = valueDate(fixingDate);
        Date d2 = maturityDate(d1);
        Time t = dayCounter_.yearFraction(d1, d2);
        QL_REQUIRE(t>0.0,
                   "\n cannot calculate forward rate between " <<
                   d1 << " and " << d2 <<
                   ":\n non positive time (" << t <<
                   ") using " << dayCounter_.name() << " daycounter");
        return forecastFixing(d1, d2, t);
    }

    Date IborIndex::maturityDate(const Date& valueDate) const {
        return fixingCalendar().advance(valueDate,
                                        tenor_,
                                        convention_,
                                        endOfMonth_);
    }

    ext::shared_ptr<IborIndex> IborIndex::clone(Handle<YieldTermStructure> h) const {
        return ext::make_shared<IborIndex>(
                                        familyName(),
                                                      tenor(),
                                                      fixingDays(),
                                                      currency(),
                                                      fixingCalendar(),
                                                      businessDayConvention(),
                                                      endOfMonth(),
                                                      dayCounter(),
                                                      std::move(h));
    }


    OvernightIndex::OvernightIndex(std::string familyName,
                                   Natural settlementDays,
                                   const Currency& curr,
                                   const Calendar& fixCal,
                                   const DayCounter& dc,
                                   Handle<YieldTermStructure> h)
   : IborIndex(std::move(familyName), 1*Days, settlementDays, curr,
               fixCal, Following, false, dc, std::move(h)) {}

    ext::shared_ptr<IborIndex> OvernightIndex::clone(Handle<YieldTermStructure> h) const {
        return ext::shared_ptr<IborIndex>(
                                        new OvernightIndex(familyName(),
                                                           fixingDays(),
                                                           currency(),
                                                           fixingCalendar(),
                                                           dayCounter(),
                                                           std::move(h)));
    }

}
