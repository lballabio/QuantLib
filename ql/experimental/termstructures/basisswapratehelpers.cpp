/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl
 Copyright (C) 2026 Kyrylo Protsenko

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

#include <ql/experimental/termstructures/quotesensitivitycalculator.hpp>
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
        DateGeneration::Rule rule,
        Integer paymentLag,
        StubIndexSelection baseStubIndexSelection,
        StubIndexSelection otherStubIndexSelection)
    : RelativeDateRateHelper(basis), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      discountHandle_(std::move(discountHandle)), bootstrapBaseCurve_(bootstrapBaseCurve),
      useIndexedCoupons_(useIndexedCoupons), rule_(rule), paymentLag_(paymentLag),
      baseStubIndexSelection_(std::move(baseStubIndexSelection)),
      otherStubIndexSelection_(std::move(otherStubIndexSelection)) {

        QL_REQUIRE(baseIndex, "null base ibor index");
        QL_REQUIRE(otherIndex, "null other ibor index");

        // stub candidates keep their own forwarding curves, so they cannot
        // track the curve being bootstrapped
        QL_REQUIRE(bootstrapBaseCurve_ ? baseStubIndexSelection_.empty()
                                       : otherStubIndexSelection_.empty(),
                   "stub index conventions are not supported on the leg "
                   "whose forecast curve is being bootstrapped");

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
        for (const auto& candidate : baseStubIndexSelection_.indices())
            registerWith(candidate);
        for (const auto& candidate : otherStubIndexSelection_.indices())
            registerWith(candidate);

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
            .withPaymentLag(paymentLag_)
            .withIndexedCoupons(useIndexedCoupons_)
            .withStubIndexSelection(baseStubIndexSelection_);
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
            .withPaymentLag(paymentLag_)
            .withIndexedCoupons(useIndexedCoupons_)
            .withStubIndexSelection(otherStubIndexSelection_);
        auto lastOtherCoupon = ext::dynamic_pointer_cast<IborCoupon>(otherLeg.back());

        maturityDate_ = std::max(baseSchedule.endDate(), otherSchedule.endDate());

        Date lastPaymentDate = std::max(baseLeg.back()->date(), otherLeg.back()->date());
        latestRelevantDate_ = std::max({maturityDate_, lastPaymentDate,
                                        lastBaseCoupon->fixingEndDate(),
                                        lastOtherCoupon->fixingEndDate()});
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

    ImpliedQuoteSensitivities IborIborBasisSwapRateHelper::impliedQuoteSensitivitiesByCurve() const {
        if (termStructure_ == nullptr || discountHandle_.empty())
            return {};
        // the bootstrapped curve forecasts one leg only
        return detail::fairBasisSensitivities(
            swap_->leg(0), swap_->leg(1), **discountHandle_);
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
        std::optional<bool> useIndexedCoupons,
        DateGeneration::Rule rule,
        RateAveraging::Type averagingMethod,
        bool telescopicValueDates,
        bool basisOnIborLeg,
        StubIndexSelection iborStubIndexSelection)
    : RelativeDateRateHelper(basis), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      discountHandle_(std::move(discountHandle)), bootstrapBaseCurve_(bootstrapBaseCurve),
      paymentLag_(paymentLag), overnightPaymentFrequency_(overnightPaymentFrequency),
      useIndexedCoupons_(useIndexedCoupons), rule_(rule), averagingMethod_(averagingMethod),
      telescopicValueDates_(telescopicValueDates), basisOnIborLeg_(basisOnIborLeg),
      iborStubIndexSelection_(std::move(iborStubIndexSelection)) {

        QL_REQUIRE(baseIndex, "null base overnight index");
        QL_REQUIRE(otherIndex, "null other ibor index");

        // stub candidates keep their own forwarding curves, so they cannot
        // track the ibor curve if that is the one being bootstrapped
        QL_REQUIRE(bootstrapBaseCurve_ || iborStubIndexSelection_.empty(),
                   "stub index conventions are not supported on the leg "
                   "whose forecast curve is being bootstrapped");

        // we need to clone the index whose forecast curve we want to bootstrap
        // and copy the other one
        if (bootstrapBaseCurve_) {
            baseIndex_ = ext::dynamic_pointer_cast<OvernightIndex>(
                baseIndex->clone(termStructureHandle_));
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
        for (const auto& candidate : iborStubIndexSelection_.indices())
            registerWith(candidate);

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
            .withPaymentLag(paymentLag_)
            .withTelescopicValueDates(
                telescopicValueDates_ && averagingMethod_ == RateAveraging::Compound)
            .withAveragingMethod(averagingMethod_);
        auto lastBaseCoupon =
            ext::dynamic_pointer_cast<OvernightIndexedCoupon>(baseLeg.back());

        // an ibor leg pays one coupon per fixing, so its payment frequency
        // is the tenor of its own index
        Schedule iborSchedule =
            MakeSchedule().from(earliestDate_).to(terminationDate)
            .withTenor(otherIndex_->tenor())
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .withRule(rule_);

        Leg otherLeg = IborLeg(iborSchedule, otherIndex_)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag_)
            .withIndexedCoupons(useIndexedCoupons_)
            .withStubIndexSelection(iborStubIndexSelection_);
        auto lastOtherCoupon = ext::dynamic_pointer_cast<IborCoupon>(otherLeg.back());

        maturityDate_ = std::max(overnightSchedule.endDate(), iborSchedule.endDate());

        // the payment lag can push the last payment past the maturity date,
        // in which case the discount curve is needed up to that date
        Date lastPaymentDate = std::max(baseLeg.back()->date(), otherLeg.back()->date());
        Date lastBaseFixingEndDate = baseIndex_->maturityDate(
            baseIndex_->valueDate(lastBaseCoupon->fixingDate()));

        latestRelevantDate_ = std::max({maturityDate_, lastPaymentDate,
                                        lastBaseFixingEndDate,
                                        lastOtherCoupon->fixingEndDate()});
        pillarDate_ = latestRelevantDate_;

        swap_ = ext::make_shared<Swap>(baseLeg, otherLeg);
        swap_->setPricingEngine(
            ext::make_shared<DiscountingSwapEngine>(discountRelinkableHandle_));
    }

    void OvernightIborBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed---the index is not lazy
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OvernightIborBasisSwapRateHelper::impliedQuote() const {
        swap_->deepUpdate();
        Size leg = basisOnIborLeg_ ? 1 : 0;
        return - (swap_->NPV() / swap_->legBPS(leg)) * 1.0e-4;
    }

    ImpliedQuoteSensitivities OvernightIborBasisSwapRateHelper::impliedQuoteSensitivitiesByCurve() const {
        if (termStructure_ == nullptr || discountRelinkableHandle_.empty())
            return {};
        // the margin is solved on the leg whose annuity impliedQuote() uses
        if (basisOnIborLeg_)
            return detail::fairBasisSensitivities(
                swap_->leg(1), swap_->leg(0), **discountRelinkableHandle_);
        return detail::fairBasisSensitivities(
            swap_->leg(0), swap_->leg(1), **discountRelinkableHandle_);
    }

    void OvernightIborBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightIborBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }



    OvernightOvernightBasisSwapRateHelper::OvernightOvernightBasisSwapRateHelper(
        const Handle<Quote>& basis,
        const Period& tenor,
        Natural settlementDays,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<OvernightIndex>& baseIndex,
        const ext::shared_ptr<OvernightIndex>& otherIndex,
        Handle<YieldTermStructure> discountHandle,
        bool bootstrapBaseCurve,
        Integer paymentLag,
        Frequency paymentFrequency,
        RateAveraging::Type baseAveragingMethod,
        RateAveraging::Type otherAveragingMethod,
        bool telescopicValueDates,
        DateGeneration::Rule rule)
    : RelativeDateRateHelper(basis), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      discountHandle_(std::move(discountHandle)), bootstrapBaseCurve_(bootstrapBaseCurve),
      paymentLag_(paymentLag), paymentFrequency_(paymentFrequency),
      baseAveragingMethod_(baseAveragingMethod),
      otherAveragingMethod_(otherAveragingMethod),
      telescopicValueDates_(telescopicValueDates), rule_(rule) {

        QL_REQUIRE(baseIndex, "null base overnight index");
        QL_REQUIRE(otherIndex, "null other overnight index");

        // We need to clone the index whose forecast curve we want to
        // bootstrap and copy the other one.
        if (bootstrapBaseCurve_) {
            baseIndex_ = ext::dynamic_pointer_cast<OvernightIndex>(
                baseIndex->clone(termStructureHandle_));
            baseIndex_->unregisterWith(termStructureHandle_);
            otherIndex_ = otherIndex;
        } else {
            baseIndex_ = baseIndex;
            otherIndex_ = ext::dynamic_pointer_cast<OvernightIndex>(
                otherIndex->clone(termStructureHandle_));
            otherIndex_->unregisterWith(termStructureHandle_);
        }

        registerWith(baseIndex_);
        registerWith(otherIndex_);
        registerWith(discountHandle_);

        OvernightOvernightBasisSwapRateHelper::initializeDates();
    }

    void OvernightOvernightBasisSwapRateHelper::initializeDates() {
        Date today = Settings::instance().evaluationDate();
        earliestDate_ = calendar_.advance(today, settlementDays_ * Days, Following);
        // unadjusted, to avoid a spurious stub
        Date terminationDate = earliestDate_ + tenor_;

        Schedule schedule =
            MakeSchedule().from(earliestDate_).to(terminationDate)
            .withFrequency(paymentFrequency_)
            .withCalendar(calendar_)
            .withConvention(convention_)
            .endOfMonth(endOfMonth_)
            .withRule(rule_);

        maturityDate_ = schedule.endDate();

        Leg baseLeg = OvernightLeg(schedule, baseIndex_)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag_)
            .withTelescopicValueDates(
                telescopicValueDates_ && baseAveragingMethod_ == RateAveraging::Compound)
            .withAveragingMethod(baseAveragingMethod_);

        Leg otherLeg = OvernightLeg(schedule, otherIndex_)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag_)
            .withTelescopicValueDates(
                telescopicValueDates_ && otherAveragingMethod_ == RateAveraging::Compound)
            .withAveragingMethod(otherAveragingMethod_);

        auto lastBaseCoupon =
            ext::dynamic_pointer_cast<OvernightIndexedCoupon>(baseLeg.back());
        auto lastOtherCoupon =
            ext::dynamic_pointer_cast<OvernightIndexedCoupon>(otherLeg.back());

        Date lastPaymentDate = std::max(baseLeg.back()->date(), otherLeg.back()->date());
        Date lastBaseFixingEndDate = baseIndex_->maturityDate(
            baseIndex_->valueDate(lastBaseCoupon->fixingDate()));
        Date lastOtherFixingEndDate = otherIndex_->maturityDate(
            otherIndex_->valueDate(lastOtherCoupon->fixingDate()));

        latestRelevantDate_ = std::max({maturityDate_, lastPaymentDate,
                                        lastBaseFixingEndDate,
                                        lastOtherFixingEndDate});
        pillarDate_ = latestRelevantDate_;

        swap_ = ext::make_shared<Swap>(baseLeg, otherLeg);
        swap_->setPricingEngine(
            ext::make_shared<DiscountingSwapEngine>(discountRelinkableHandle_));
    }

    void OvernightOvernightBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // Do not set the relinkable handle as an observer: force
        // recalculation when needed; the index is not lazy.
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OvernightOvernightBasisSwapRateHelper::impliedQuote() const {
        swap_->deepUpdate();
        return -(swap_->NPV() / swap_->legBPS(0)) * 1.0e-4;
    }

    ImpliedQuoteSensitivities OvernightOvernightBasisSwapRateHelper::impliedQuoteSensitivitiesByCurve() const {
        if (termStructure_ == nullptr || discountRelinkableHandle_.empty())
            return {};
        return detail::fairBasisSensitivities(
            swap_->leg(0), swap_->leg(1), **discountRelinkableHandle_);
    }

    void OvernightOvernightBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightOvernightBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
