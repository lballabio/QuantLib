/*
 Copyright (C) 2006 Ferdinando Ametrano
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
#include <ql/Instruments/makevanillaswap.hpp>

#include <sstream>


namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    SwapIndex::SwapIndex(const std::string& familyName,
                         Integer years,
                         Integer settlementDays,
                         const Currency& currency,
                         const Calendar& calendar,
                         Frequency fixedLegFrequency,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         const boost::shared_ptr<IborIndex>& iborIndex)
    : InterestRateIndex(familyName, years*Years, settlementDays,
                        currency, calendar, fixedLegDayCounter),
      tenor_(years*Years), iborIndex_(iborIndex),
      fixedLegTenor_(Period(fixedLegFrequency)),
      fixedLegConvention_(fixedLegConvention) {
        registerWith(iborIndex_);
    }

    SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Integer settlementDays,
                         Currency currency,
                         const Calendar& calendar,
                         Frequency fixedLegFrequency,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         const boost::shared_ptr<IborIndex>& iborIndex)
    : InterestRateIndex(familyName, tenor, settlementDays,
                        currency, calendar, fixedLegDayCounter),
      tenor_(tenor), iborIndex_(iborIndex),
      fixedLegTenor_(Period(fixedLegFrequency)),
      fixedLegConvention_(fixedLegConvention) {
        registerWith(iborIndex_);
    }
    #endif

    SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Integer settlementDays,
                         Currency currency,
                         const Calendar& calendar,
                         const Period& fixedLegTenor,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         const boost::shared_ptr<IborIndex>& iborIndex)
    : InterestRateIndex(familyName, tenor, settlementDays,
                        currency, calendar, fixedLegDayCounter),
      tenor_(tenor), iborIndex_(iborIndex),
      fixedLegTenor_(fixedLegTenor),
      fixedLegConvention_(fixedLegConvention) {
        registerWith(iborIndex_);
    }

    Rate SwapIndex::forecastFixing(const Date& fixingDate) const {
        return underlyingSwap(fixingDate)->fairRate();
    }

    boost::shared_ptr<VanillaSwap> SwapIndex::underlyingSwap(
                                               const Date& fixingDate) const {
        QL_REQUIRE(iborIndex_, "no index set");
        QL_REQUIRE(iborIndex_->termStructure(),
                   "no forecasting term structure set to " <<
                   iborIndex_->name());
        return MakeVanillaSwap(tenor_, iborIndex_, 0.0)
            .withEffectiveDate(valueDate(fixingDate))
            .withFixedLegCalendar(calendar_)
            .withFixedLegDayCount(dayCounter_)
            .withFixedLegTenor(fixedLegTenor_)
            .withFixedLegConvention(fixedLegConvention_)
            .withFixedLegTerminationDateConvention(fixedLegConvention_);
    }

    Schedule SwapIndex::fixedRateSchedule(const Date& fixingDate) const {

        Date start = calendar_.advance(fixingDate, fixingDays_, Days);
        Date end = calendar_.advance(start, tenor_);

        return Schedule(start, end, fixedLegTenor_, calendar_,
                        fixedLegConvention_, fixedLegConvention_,
                        false, false);
    }

    #ifdef QL_DISABLE_DEPRECATED
    Date SwapIndex::maturityDate(const Date& valueDate) const {
        return calendar_.advance(valueDate, tenor_, Unadjusted, false);
    }
    #endif


}

