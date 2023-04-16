/*
 Copyright (C) 2006, 2009 Ferdinando Ametrano
 Copyright (C) 2006, 2007, 2009 StatPro Italia srl

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

#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/time/schedule.hpp>
#include <sstream>
#include <utility>

namespace QuantLib {

    SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         const Currency& currency,
                         const Calendar& fixingCalendar,
                         const Period& fixedLegTenor,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         std::shared_ptr<IborIndex> iborIndex)
    : InterestRateIndex(
          familyName, tenor, settlementDays, currency, fixingCalendar, fixedLegDayCounter),
      tenor_(tenor), iborIndex_(std::move(iborIndex)), fixedLegTenor_(fixedLegTenor),
      fixedLegConvention_(fixedLegConvention), exogenousDiscount_(false) {
        registerWith(iborIndex_);
    }

    SwapIndex::SwapIndex(const std::string& familyName,
                         const Period& tenor,
                         Natural settlementDays,
                         const Currency& currency,
                         const Calendar& fixingCalendar,
                         const Period& fixedLegTenor,
                         BusinessDayConvention fixedLegConvention,
                         const DayCounter& fixedLegDayCounter,
                         std::shared_ptr<IborIndex> iborIndex,
                         Handle<YieldTermStructure> discount)
    : InterestRateIndex(
          familyName, tenor, settlementDays, currency, fixingCalendar, fixedLegDayCounter),
      tenor_(tenor), iborIndex_(std::move(iborIndex)), fixedLegTenor_(fixedLegTenor),
      fixedLegConvention_(fixedLegConvention), exogenousDiscount_(true),
      discount_(std::move(discount)) {
        registerWith(iborIndex_);
        registerWith(discount_);
    }

    Handle<YieldTermStructure> SwapIndex::forwardingTermStructure() const {
        return iborIndex_->forwardingTermStructure();
    }

    Handle<YieldTermStructure> SwapIndex::discountingTermStructure() const {
        return discount_;  // empty if not exogenous
    }

    Rate SwapIndex::forecastFixing(const Date& fixingDate) const {
        return underlyingSwap(fixingDate)->fairRate();
    }

    std::shared_ptr<VanillaSwap>
    SwapIndex::underlyingSwap(const Date& fixingDate) const {

        QL_REQUIRE(fixingDate!=Date(), "null fixing date");

        // caching mechanism
        if (lastFixingDate_!=fixingDate) {
            Rate fixedRate = 0.0;
            if (exogenousDiscount_)
                lastSwap_ = MakeVanillaSwap(tenor_, iborIndex_, fixedRate)
                    .withEffectiveDate(valueDate(fixingDate))
                    .withFixedLegCalendar(fixingCalendar())
                    .withFixedLegDayCount(dayCounter_)
                    .withFixedLegTenor(fixedLegTenor_)
                    .withFixedLegConvention(fixedLegConvention_)
                    .withFixedLegTerminationDateConvention(fixedLegConvention_)
                    .withDiscountingTermStructure(discount_);
            else
                lastSwap_ = MakeVanillaSwap(tenor_, iborIndex_, fixedRate)
                    .withEffectiveDate(valueDate(fixingDate))
                    .withFixedLegCalendar(fixingCalendar())
                    .withFixedLegDayCount(dayCounter_)
                    .withFixedLegTenor(fixedLegTenor_)
                    .withFixedLegConvention(fixedLegConvention_)
                    .withFixedLegTerminationDateConvention(fixedLegConvention_);
            lastFixingDate_ = fixingDate;
        }
        return lastSwap_;
    }

    Date SwapIndex::maturityDate(const Date& valueDate) const {
        Date fixDate = fixingDate(valueDate);
        return underlyingSwap(fixDate)->maturityDate();
    }

    std::shared_ptr<SwapIndex>
    SwapIndex::clone(const Handle<YieldTermStructure>& forwarding) const {

        if (exogenousDiscount_)
            return std::shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor(),
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex_->clone(forwarding),
                          discount_));
        else
            return std::shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor(),
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex_->clone(forwarding)));
    }

    std::shared_ptr<SwapIndex>
    SwapIndex::clone(const Handle<YieldTermStructure>& forwarding,
                     const Handle<YieldTermStructure>& discounting) const {
        return std::shared_ptr<SwapIndex>(new
             SwapIndex(familyName(),
                       tenor(),
                       fixingDays(),
                       currency(),
                       fixingCalendar(),
                       fixedLegTenor(),
                       fixedLegConvention(),
                       dayCounter(),
                       iborIndex_->clone(forwarding),
                       discounting));
    }

    std::shared_ptr<SwapIndex>
    SwapIndex::clone(const Period& tenor) const {

        if (exogenousDiscount_)
            return std::shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor,
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex(),
                          discountingTermStructure()));
        else
            return std::shared_ptr<SwapIndex>(new
                SwapIndex(familyName(),
                          tenor,
                          fixingDays(),
                          currency(),
                          fixingCalendar(),
                          fixedLegTenor(),
                          fixedLegConvention(),
                          dayCounter(),
                          iborIndex()));

    }

    OvernightIndexedSwapIndex::OvernightIndexedSwapIndex(
        const std::string& familyName,
        const Period& tenor,
        Natural settlementDays,
        const Currency& currency,
        const std::shared_ptr<OvernightIndex>& overnightIndex,
        bool telescopicValueDates,
        RateAveraging::Type averagingMethod)
    : SwapIndex(familyName,
                tenor,
                settlementDays,
                currency,
                overnightIndex->fixingCalendar(),
                1 * Years,
                ModifiedFollowing,
                overnightIndex->dayCounter(),
                overnightIndex),
      overnightIndex_(overnightIndex), 
      telescopicValueDates_(telescopicValueDates), 
      averagingMethod_(averagingMethod) {}


    std::shared_ptr<OvernightIndexedSwap>
    OvernightIndexedSwapIndex::underlyingSwap(const Date& fixingDate) const {

        QL_REQUIRE(fixingDate!=Date(), "null fixing date");

        // caching mechanism
        if (lastFixingDate_!=fixingDate) {
            Rate fixedRate = 0.0;
            lastSwap_ = MakeOIS(tenor_, overnightIndex_, fixedRate)
                .withEffectiveDate(valueDate(fixingDate))
                .withFixedLegDayCount(dayCounter_)
                .withTelescopicValueDates(telescopicValueDates_)
                .withAveragingMethod(averagingMethod_);
            lastFixingDate_ = fixingDate;
        }
        return lastSwap_;
    }

}
