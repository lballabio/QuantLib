/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/Indexes/swapindex.hpp>
#include <ql/Instruments/all.hpp>

#include <sstream>


namespace QuantLib {

    SwapIndex::SwapIndex(const std::string& familyName,
                         Integer years,
                         Integer settlementDays,
                         Currency currency,
                         const Calendar& calendar,
                         Frequency fixedLegFrequency,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         const boost::shared_ptr<Xibor>& iborIndex)
    : InterestRateIndex(familyName, Period(years, Years), settlementDays,
                        currency, calendar, fixedLegDayCounter),
      years_(years), iborIndex_(iborIndex),
      fixedLegFrequency_(fixedLegFrequency),
      fixedLegConvention_(fixedLegConvention)
    {
        registerWith(iborIndex_);
    }

    Rate SwapIndex::forecastFixing(const Date& fixingDate) const {
        return underlyingSwap(fixingDate)->fairRate();
    }

    boost::shared_ptr<VanillaSwap> SwapIndex::underlyingSwap(
                                               const Date& fixingDate) const {
        QL_REQUIRE(iborIndex_, "no index set");
        QL_REQUIRE(iborIndex_->termStructure(), "no term structure set");
        Date start = calendar_.advance(fixingDate, settlementDays_, Days);
        return MakeVanillaSwap(start, Period(years_, Years), calendar_, 0.0,
                               iborIndex_,
                               Handle<YieldTermStructure>(
                                   iborIndex_->termStructure()))
            .withFixedLegDayCount(dayCounter_)
            .withFixedLegTenor(Period(fixedLegFrequency_))
            .withFixedLegConvention(fixedLegConvention_)
            .withFixedLegTerminationDateConvention(fixedLegConvention_);
    }

    Schedule SwapIndex::fixedRateSchedule(const Date& fixingDate) const {

        Date start = calendar_.advance(fixingDate, settlementDays_,Days);
        Date end = calendar_.advance(start,years_,Years);

        return Schedule(start, end, Period(fixedLegFrequency_), calendar_,
                        fixedLegConvention_, fixedLegConvention_,
                        false, false);
    }

}

