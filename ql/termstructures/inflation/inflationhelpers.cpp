/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    ZeroCouponInflationSwapHelper::ZeroCouponInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& maturity,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        const ext::shared_ptr<ZeroInflationIndex>& zii,
        CPI::InterpolationType observationInterpolation)
    : ZeroCouponInflationSwapHelper(
        quote, swapObsLag, Date(), maturity, std::move(calendar), paymentConvention,
        std::move(dayCounter), zii, observationInterpolation) {}

    ZeroCouponInflationSwapHelper::ZeroCouponInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& startDate,
        const Date& endDate,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        const ext::shared_ptr<ZeroInflationIndex>& zii,
        CPI::InterpolationType observationInterpolation)
    : ZeroCouponInflationSwapHelper(
        quote, swapObsLag, startDate, endDate, std::move(calendar), paymentConvention,
        std::move(dayCounter), zii, observationInterpolation,
        // any nominal term structure will give the same result;
        // when calculating the fair rate, the equal discount factors
        // for the payments on the two legs will cancel out.
        Handle<YieldTermStructure>(ext::make_shared<FlatForward>(0, NullCalendar(), 0.0, dayCounter))) {}

    ZeroCouponInflationSwapHelper::ZeroCouponInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& maturity,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        const ext::shared_ptr<ZeroInflationIndex>& zii,
        CPI::InterpolationType observationInterpolation,
        Handle<YieldTermStructure> nominalTermStructure)
    : ZeroCouponInflationSwapHelper(
        quote, swapObsLag, Date(), maturity, std::move(calendar), paymentConvention,
        dayCounter, zii, observationInterpolation, std::move(nominalTermStructure)) {}

    ZeroCouponInflationSwapHelper::ZeroCouponInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& startDate,
        const Date& endDate,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        const ext::shared_ptr<ZeroInflationIndex>& zii,
        CPI::InterpolationType observationInterpolation,
        Handle<YieldTermStructure> nominalTermStructure)
    : RelativeDateBootstrapHelper<ZeroInflationTermStructure>(quote, startDate == Date()),
      swapObsLag_(swapObsLag), startDate_(startDate), maturity_(endDate),
      calendar_(std::move(calendar)), paymentConvention_(paymentConvention),
      dayCounter_(std::move(dayCounter)), observationInterpolation_(observationInterpolation),
      nominalTermStructure_(std::move(nominalTermStructure)) {
        zii_ = zii->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        zii_->unregisterWith(termStructureHandle_);

        auto fixingPeriod = inflationPeriod(maturity_ - swapObsLag_, zii_->frequency());
        auto interpolationPeriod = inflationPeriod(maturity_, zii_->frequency());

        if (detail::CPI::isInterpolated(observationInterpolation_) && maturity_ > interpolationPeriod.first) {
            // if interpolated, we need to cover the end of the interpolation period
            earliestDate_ = fixingPeriod.first;
            latestDate_ = fixingPeriod.second + 1;
        } else {
            // if not interpolated, the date of the initial fixing is enough
            earliestDate_ = fixingPeriod.first;
            latestDate_ = fixingPeriod.first;
        }

        // check that the observation lag of the swap
        // is compatible with the availability lag of the index AND
        // it's interpolation (assuming the start day is spot)
        if (detail::CPI::isInterpolated(observationInterpolation_)) {
            Period pShift(zii_->frequency());
            QL_REQUIRE(swapObsLag_ - pShift >= zii_->availabilityLag(),
                       "inconsistency between swap observation lag "
                           << swapObsLag_ << ", index period " << pShift << " and index availability "
                           << zii_->availabilityLag() << ": need (obsLag-index period) >= availLag");
        }

        registerWith(zii_);
        registerWith(nominalTermStructure_);
        ZeroCouponInflationSwapHelper::initializeDates();
    }

    QL_DEPRECATED_ENABLE_WARNING


    Real ZeroCouponInflationSwapHelper::impliedQuote() const {
        zciis_->deepUpdate();
        return zciis_->fairRate();
    }

    void ZeroCouponInflationSwapHelper::initializeDates() {
        zciis_ = ext::make_shared<ZeroCouponInflationSwap>(
            Swap::Payer, 1.0, updateDates_ ? evaluationDate_ : startDate_, maturity_, calendar_,
            paymentConvention_, dayCounter_, 0.0, zii_, swapObsLag_,
            observationInterpolation_);
        // The instrument takes a standard discounting swap engine.
        // The inflation-related work is done by the coupons.
        zciis_->setPricingEngine(
            ext::make_shared<DiscountingSwapEngine>(nominalTermStructure_));
    }

    void ZeroCouponInflationSwapHelper::setTermStructure(ZeroInflationTermStructure* z) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<ZeroInflationTermStructure> temp(z, null_deleter());
        termStructureHandle_.linkTo(std::move(temp), observer);

        RelativeDateBootstrapHelper<ZeroInflationTermStructure>::setTermStructure(z);
    }


    YearOnYearInflationSwapHelper::YearOnYearInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& maturity,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        const ext::shared_ptr<YoYInflationIndex>& yii,
        CPI::InterpolationType interpolation,
        Handle<YieldTermStructure> nominalTermStructure)
    : YearOnYearInflationSwapHelper(
        quote, swapObsLag, Date(), maturity, std::move(calendar), paymentConvention,
        std::move(dayCounter), yii, interpolation, std::move(nominalTermStructure)) {}

    YearOnYearInflationSwapHelper::YearOnYearInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& startDate,
        const Date& endDate,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        const ext::shared_ptr<YoYInflationIndex>& yii,
        CPI::InterpolationType interpolation,
        Handle<YieldTermStructure> nominalTermStructure)
    : RelativeDateBootstrapHelper<YoYInflationTermStructure>(quote, startDate == Date()),
      swapObsLag_(swapObsLag), startDate_(startDate), maturity_(endDate),
      calendar_(std::move(calendar)), paymentConvention_(paymentConvention),
      dayCounter_(std::move(dayCounter)), interpolation_(interpolation),
      nominalTermStructure_(std::move(nominalTermStructure)) {
        yii_ = yii->clone(termStructureHandle_);
        // We want to be notified of changes of fixings, but we don't
        // want notifications from termStructureHandle_ (they would
        // interfere with bootstrapping.)
        yii_->unregisterWith(termStructureHandle_);

        auto fixingPeriod = inflationPeriod(maturity_ - swapObsLag_, yii_->frequency());
        auto interpolationPeriod = inflationPeriod(maturity_, yii_->frequency());

        if (detail::CPI::isInterpolated(interpolation_, yii_) && maturity_ > interpolationPeriod.first) {
            // if interpolated, we need to cover the end of the interpolation period
            earliestDate_ = fixingPeriod.first;
            latestDate_ = fixingPeriod.second + 1;
        } else {
            // if not interpolated, the date of the initial fixing is enough
            earliestDate_ = fixingPeriod.first;
            latestDate_ = fixingPeriod.first;
        }

        // check that the observation lag of the swap
        // is compatible with the availability lag of the index AND
        // its interpolation (assuming the start day is spot)
        if (detail::CPI::isInterpolated(interpolation_, yii_)) {
            Period pShift(yii_->frequency());
            QL_REQUIRE(swapObsLag_ - pShift >= yii_->availabilityLag(),
                       "inconsistency between swap observation lag "
                       << swapObsLag_ << ", index period " << pShift << " and index availability "
                       << yii_->availabilityLag() << ": need (obsLag-index period) >= availLag");
        }

        registerWith(yii_);
        registerWith(nominalTermStructure_);
        YearOnYearInflationSwapHelper::initializeDates();
    }

    Real YearOnYearInflationSwapHelper::impliedQuote() const {
        yyiis_->deepUpdate();
        return yyiis_->fairRate();
    }

    void YearOnYearInflationSwapHelper::initializeDates() {
        // always works because tenor is always 1 year so
        // no problem with different days-in-month
        Schedule fixedSchedule = MakeSchedule()
                                     .from(updateDates_ ? evaluationDate_ : startDate_)
                                     .to(maturity_)
                                     .withTenor(1 * Years)
                                     .withConvention(Unadjusted)
                                     .withCalendar(calendar_) // fixed leg gets cal from sched
                                     .backwards();
        const Schedule& yoySchedule = fixedSchedule;

        yyiis_ = ext::make_shared<YearOnYearInflationSwap>(
            Swap::Payer, 1.0, fixedSchedule, 0.0, dayCounter_,
            yoySchedule, yii_, swapObsLag_, interpolation_,
            0.0, dayCounter_, calendar_, paymentConvention_);

        // The instrument takes a standard discounting swap engine.
        // The inflation-related work is done by the coupons.
        yyiis_->setPricingEngine(
            ext::make_shared<DiscountingSwapEngine>(nominalTermStructure_));
    }

    void YearOnYearInflationSwapHelper::setTermStructure(YoYInflationTermStructure* y) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YoYInflationTermStructure> temp(y, null_deleter());
        termStructureHandle_.linkTo(std::move(temp), observer);

        RelativeDateBootstrapHelper<YoYInflationTermStructure>::setTermStructure(y);
    }

}
