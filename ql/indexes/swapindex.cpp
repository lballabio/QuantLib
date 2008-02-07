/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <sstream>

namespace QuantLib {

    SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
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

    Handle<YieldTermStructure> SwapIndex::termStructure() const {
        return iborIndex_->termStructure();
    }

    Rate SwapIndex::forecastFixing(const Date& fixingDate) const {
        return underlyingSwap(fixingDate)->fairRate();
    }

    boost::shared_ptr<VanillaSwap> SwapIndex::underlyingSwap(
                                               const Date& fixingDate) const {
        Rate fixedRate = 0.0;
        return MakeVanillaSwap(tenor_, iborIndex_, fixedRate)
            .withEffectiveDate(valueDate(fixingDate))
            .withFixedLegCalendar(fixingCalendar())
            .withFixedLegDayCount(dayCounter_)
            .withFixedLegTenor(fixedLegTenor_)
            .withFixedLegConvention(fixedLegConvention_)
            .withFixedLegTerminationDateConvention(fixedLegConvention_);
    }

    Date SwapIndex::maturityDate(const Date& valueDate) const {
        Date fixDate = fixingDate(valueDate);
        return underlyingSwap(fixDate)->maturityDate();
    }

}
