/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Zain Mughal

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/makemultipleresetsswap.hpp>
#include <ql/instruments/simplifynotificationgraph.hpp>
#include <ql/termstructures/yield/multipleresetsswaphelper.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    MultipleResetsSwapRateHelper::MultipleResetsSwapRateHelper(
        Natural settlementDays,
        const Period& tenor,
        const std::variant<Rate, Handle<Quote>>& fixedRate,
        const ext::shared_ptr<IborIndex>& iborIndex,
        Size resetsPerCoupon,
        Handle<YieldTermStructure> discountingCurve,
        RateAveraging::Type averagingMethod,
        Spread spread,
        Frequency fixedFrequency,
        DayCounter fixedDayCount,
        BusinessDayConvention fixedConvention)
    : RelativeDateRateHelper(fixedRate),
      settlementDays_(settlementDays), tenor_(tenor),
      resetsPerCoupon_(resetsPerCoupon),
      averagingMethod_(averagingMethod), spread_(spread),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(!fixedDayCount.empty() ? std::move(fixedDayCount) :
                                              iborIndex->dayCounter()),
      fixedConvention_(fixedConvention),
      discountHandle_(std::move(discountingCurve)) {

        // Clone the index so it forwards rates from termStructureHandle_,
        // but don't register the clone as an observer of termStructureHandle_
        // (that would interfere with bootstrapping).
        iborIndex_ = ext::dynamic_pointer_cast<IborIndex>(
            iborIndex->clone(termStructureHandle_));
        iborIndex_->unregisterWith(termStructureHandle_);

        registerWith(iborIndex_);
        registerWith(discountHandle_);
        initializeDates();
    }

    void MultipleResetsSwapRateHelper::initializeDates() {
        swap_ = MakeMultipleResetsSwap(tenor_, iborIndex_, resetsPerCoupon_, 0.0)
            .withSettlementDays(settlementDays_)
            .withFixedLegFrequency(fixedFrequency_)
            .withFixedLegDayCount(fixedDayCount_)
            .withFixedLegConvention(fixedConvention_)
            .withFloatingLegSpread(spread_)
            .withAveragingMethod(averagingMethod_)
            .withDiscountingTermStructure(discountRelinkableHandle_);

        simplifyNotificationGraph(*swap_, true);

        earliestDate_ = swap_->startDate();
        latestRelevantDate_ = latestDate_ =
            std::max(swap_->fixedLeg().back()->date(),
                     swap_->floatingLeg().back()->date());
    }

    void MultipleResetsSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        bool observer = false;
        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);
        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);
        RelativeDateRateHelper::setTermStructure(t);
    }

    Real MultipleResetsSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        swap_->deepUpdate();
        return swap_->fairRate();
    }

    void MultipleResetsSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<MultipleResetsSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
