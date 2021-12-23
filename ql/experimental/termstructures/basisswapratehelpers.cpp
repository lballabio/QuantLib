/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/experimental/termstructures/basisswapratehelpers.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    IborIborBasisSwapRateHelper::IborIborBasisSwapRateHelper(
        const Handle<Quote>& basis,
        const Period& tenor,
        Natural settlementDays,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<IborIndex>& baseIndex,
        const ext::shared_ptr<IborIndex>& otherIndex,
        const Handle<YieldTermStructure>& discountHandle,
        bool bootstrapBaseCurve)
    : RelativeDateRateHelper(basis), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(std::move(calendar))), convention_(convention), endOfMonth_(endOfMonth),
      discountHandle_(discountHandle), bootstrapBaseCurve_(bootstrapBaseCurve) {

        // we need to clone the index whose forecast curve we want to bootstrap
        // and copy the other one
        if (bootstrapBaseCurve_) {
            baseIndex_ = baseIndex->clone(termStructureHandle_);
            baseIndex_->unregisterWith(termStructureHandle_);
            otherIndex_ = otherIndex;
        } else {
            baseIndex_ = baseIndex;
            otherIndex_ = otherIndex->clone(termStructureHandle_);
            otherIndex_->unregisterWith(termStructureHandle_);
        }

        registerWith(baseIndex_);
        registerWith(otherIndex_);
        registerWith(discountHandle);

        initializeDates();
    }

    void IborIborBasisSwapRateHelper::initializeDates() {
        Date today = Settings::instance().evaluationDate();
        earliestDate_ = calendar_.advance(today, settlementDays_ * Days, Following);
        maturityDate_ = calendar_.advance(earliestDate_, tenor_, convention_);

        Schedule baseSchedule =
            MakeSchedule().from(earliestDate_).to(maturityDate_)
            .withTenor(baseIndex_->tenor())
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .forwards();
        Leg baseLeg = IborLeg(baseSchedule, baseIndex_).withNotionals(100.0);
        auto lastBaseCoupon = ext::dynamic_pointer_cast<IborCoupon>(baseLeg.back());

        Schedule otherSchedule =
            MakeSchedule().from(earliestDate_).to(maturityDate_)
            .withTenor(otherIndex_->tenor())
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .forwards();
        Leg otherLeg = IborLeg(otherSchedule, otherIndex_).withNotionals(100.0);
        auto lastOtherCoupon = ext::dynamic_pointer_cast<IborCoupon>(otherLeg.back());

        latestRelevantDate_ = std::max(maturityDate_,
                                       std::max(lastBaseCoupon->fixingEndDate(),
                                                lastOtherCoupon->fixingEndDate()));
        pillarDate_ = latestRelevantDate_;

        swap_ = ext::make_shared<Swap>(baseLeg, otherLeg);
        swap_->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountHandle_));
    }

    void IborIborBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed---the index is not lazy
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real IborIborBasisSwapRateHelper::impliedQuote() const {
        swap_->recalculate();
        return - (swap_->NPV() / swap_->legBPS(0)) * 1.0e-4;
    }

    void IborIborBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<IborIborBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
