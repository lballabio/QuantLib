/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
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
        Handle<YieldTermStructure> discountHandle,
        bool bootstrapBaseCurve,
        std::optional<bool> useIndexedCoupons,
        DateGeneration::Rule rule)
    : RelativeDateRateHelper(basis), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      discountHandle_(std::move(discountHandle)), bootstrapBaseCurve_(bootstrapBaseCurve),
      useIndexedCoupons_(useIndexedCoupons), rule_(rule) {

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
        registerWith(discountHandle_);

        IborIborBasisSwapRateHelper::initializeDates();
    }

    void IborIborBasisSwapRateHelper::initializeDates() {
        Date today = Settings::instance().evaluationDate();
        earliestDate_ = calendar_.advance(today, settlementDays_ * Days, Following);
        // unadjusted, to avoid a spurious stub
        Date terminationDate = earliestDate_ + tenor_;

        Schedule baseSchedule =
            MakeSchedule().from(earliestDate_).to(terminationDate)
            .withTenor(baseIndex_->tenor())
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .withRule(rule_);
        Leg baseLeg = IborLeg(baseSchedule, baseIndex_)
            .withNotionals(100.0)
            .withIndexedCoupons(useIndexedCoupons_);
        auto lastBaseCoupon = ext::dynamic_pointer_cast<IborCoupon>(baseLeg.back());

        Schedule otherSchedule =
            MakeSchedule().from(earliestDate_).to(terminationDate)
            .withTenor(otherIndex_->tenor())
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .withRule(rule_);
        Leg otherLeg = IborLeg(otherSchedule, otherIndex_)
            .withNotionals(100.0)
            .withIndexedCoupons(useIndexedCoupons_);
        auto lastOtherCoupon = ext::dynamic_pointer_cast<IborCoupon>(otherLeg.back());

        maturityDate_ = std::max(baseSchedule.endDate(), otherSchedule.endDate());

        latestRelevantDate_ = std::max(maturityDate_,
                                       std::max(lastBaseCoupon->fixingEndDate(),
                                                lastOtherCoupon->fixingEndDate()));
        pillarDate_ = latestRelevantDate_;

        swap_ = ext::make_shared<Swap>(baseLeg, otherLeg);
        swap_->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountHandle_));
    }

    void IborIborBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real IborIborBasisSwapRateHelper::impliedQuote() const {
        swap_->deepUpdate();
        return - (swap_->NPV() / swap_->legBPS(0)) * 1.0e-4;
    }

    void IborIborBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<IborIborBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }



    OvernightIborBasisSwapRateHelper::OvernightIborBasisSwapRateHelper(
        const Handle<Quote>& basis,
        const Period& tenor,
        Natural settlementDays,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<OvernightIndex>& baseIndex,
        const ext::shared_ptr<IborIndex>& otherIndex,
        Handle<YieldTermStructure> discountHandle,
        bool bootstrapBaseCurve,
        Integer paymentLag,
        std::optional<Frequency> overnightPaymentFrequency,
        std::optional<Frequency> iborPaymentFrequency,
        std::optional<bool> useIndexedCoupons,
        DateGeneration::Rule rule)
    : RelativeDateRateHelper(basis), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      discountHandle_(std::move(discountHandle)), bootstrapBaseCurve_(bootstrapBaseCurve),
      paymentLag_(paymentLag), overnightPaymentFrequency_(overnightPaymentFrequency),
      iborPaymentFrequency_(iborPaymentFrequency),
      useIndexedCoupons_(useIndexedCoupons), rule_(rule) {

        // we need to clone the index whose forecast curve we want to bootstrap
        // and copy the other one
        if (bootstrapBaseCurve_) {
            baseIndex_ = ext::dynamic_pointer_cast<OvernightIndex>(
                baseIndex->clone(termStructureHandle_));
            QL_REQUIRE(baseIndex_ != nullptr,
                       "the base index did not clone into an overnight index");
            baseIndex_->unregisterWith(termStructureHandle_);
            otherIndex_ = otherIndex;
        } else {
            baseIndex_ = baseIndex;
            otherIndex_ = otherIndex->clone(termStructureHandle_);
            otherIndex_->unregisterWith(termStructureHandle_);
        }

        registerWith(baseIndex_);
        registerWith(otherIndex_);
        registerWith(discountHandle_);

        OvernightIborBasisSwapRateHelper::initializeDates();
    }

    void OvernightIborBasisSwapRateHelper::initializeDates() {
        Date today = Settings::instance().evaluationDate();
        earliestDate_ = calendar_.advance(today, settlementDays_ * Days, Following);
        // unadjusted, to avoid a spurious stub
        Date terminationDate = earliestDate_ + tenor_;

        Period overnightTenor =
            overnightPaymentFrequency_ ? Period(*overnightPaymentFrequency_) : otherIndex_->tenor();
        Schedule overnightSchedule =
            MakeSchedule().from(earliestDate_).to(terminationDate)
            .withTenor(overnightTenor)
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .withRule(rule_);

        Leg baseLeg = OvernightLeg(overnightSchedule, baseIndex_)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag_);

        Period iborTenor =
            iborPaymentFrequency_ ? Period(*iborPaymentFrequency_) : otherIndex_->tenor();
        Schedule iborSchedule =
            MakeSchedule().from(earliestDate_).to(terminationDate)
            .withTenor(iborTenor)
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .withRule(rule_);

        Leg otherLeg = IborLeg(iborSchedule, otherIndex_)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag_)
            .withIndexedCoupons(useIndexedCoupons_);
        auto lastOtherCoupon = ext::dynamic_pointer_cast<IborCoupon>(otherLeg.back());

        maturityDate_ = std::max(overnightSchedule.endDate(), iborSchedule.endDate());

        // the payment lag can push the last payment past the maturity date,
        // in which case the discount curve is needed up to that date
        Date lastPaymentDate = std::max(baseLeg.back()->date(), otherLeg.back()->date());

        latestRelevantDate_ = std::max({maturityDate_, lastPaymentDate,
                                        lastOtherCoupon->fixingEndDate()});
        pillarDate_ = latestRelevantDate_;

        swap_ = ext::make_shared<Swap>(baseLeg, otherLeg);
        swap_->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(
            discountHandle_.empty() ? termStructureHandle_ : discountHandle_));
    }

    void OvernightIborBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed---the index is not lazy
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OvernightIborBasisSwapRateHelper::impliedQuote() const {
        swap_->deepUpdate();
        return - (swap_->NPV() / swap_->legBPS(0)) * 1.0e-4;
    }

    void OvernightIborBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightIborBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
