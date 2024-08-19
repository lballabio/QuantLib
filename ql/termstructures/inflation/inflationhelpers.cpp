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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */

#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    ZeroCouponInflationSwapHelper::ZeroCouponInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& maturity,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        ext::shared_ptr<ZeroInflationIndex> zii,
        CPI::InterpolationType observationInterpolation,
        Handle<YieldTermStructure> nominalTermStructure)
    : BootstrapHelper<ZeroInflationTermStructure>(quote), swapObsLag_(swapObsLag),
      maturity_(maturity), calendar_(std::move(calendar)), paymentConvention_(paymentConvention),
      dayCounter_(std::move(dayCounter)), zii_(std::move(zii)),
      observationInterpolation_(observationInterpolation),
      nominalTermStructure_(std::move(nominalTermStructure)) {

        std::pair<Date, Date> limStart = inflationPeriod(maturity_ - swapObsLag_, zii_->frequency());
        std::pair<Date, Date> interpolationPeriod = inflationPeriod(maturity, zii_->frequency());

        if ((detail::CPI::effectiveInterpolationType(observationInterpolation_) == CPI::Linear) &&
            (maturity > interpolationPeriod.first)) {
            // if interpolated, we need to cover the end of the interpolation period
            earliestDate_ = limStart.first;
            latestDate_ = limStart.second + 1;
        } else {
            // if not interpolated, the date of the initial fixing is enough
            earliestDate_ = limStart.first;
            latestDate_ = limStart.first;
        }

        // check that the observation lag of the swap
        // is compatible with the availability lag of the index AND
        // it's interpolation (assuming the start day is spot)
        if (detail::CPI::effectiveInterpolationType(observationInterpolation_) == CPI::Linear) {
            Period pShift(zii_->frequency());
            QL_REQUIRE(swapObsLag_ - pShift >= zii_->availabilityLag(),
                       "inconsistency between swap observation lag "
                           << swapObsLag_ << ", index period " << pShift << " and index availability "
                           << zii_->availabilityLag() << ": need (obsLag-index period) >= availLag");
        }

        registerWith(Settings::instance().evaluationDate());
        registerWith(nominalTermStructure_);
    }


    Real ZeroCouponInflationSwapHelper::impliedQuote() const {
        zciis_->deepUpdate();
        return zciis_->fairRate();
    }


    void ZeroCouponInflationSwapHelper::setTermStructure(ZeroInflationTermStructure* z) {

        BootstrapHelper<ZeroInflationTermStructure>::setTermStructure(z);

        // set up a new ZCIIS
        // but this one does NOT own its inflation term structure
        const bool own = false;
        Rate K = quote()->value();

        // The effect of the new inflation term structure is
        // felt via the effect on the inflation index
        Handle<ZeroInflationTermStructure> zits(
            ext::shared_ptr<ZeroInflationTermStructure>(z, null_deleter()), own);

        ext::shared_ptr<ZeroInflationIndex> new_zii = zii_->clone(zits);

        Real nominal = 1000000.0; // has to be something but doesn't matter what
        Date start = nominalTermStructure_->referenceDate();
        zciis_ = ext::make_shared<ZeroCouponInflationSwap>(Swap::Payer, nominal, start,
                                                 maturity_, calendar_, paymentConvention_,
                                                 dayCounter_, K, // fixed side & fixed rate
                                                 new_zii, swapObsLag_, observationInterpolation_);
        // Because very simple instrument only takes
        // standard discounting swap engine.
        zciis_->setPricingEngine(
            ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(nominalTermStructure_)));
    }


    YearOnYearInflationSwapHelper::YearOnYearInflationSwapHelper(
        const Handle<Quote>& quote,
        const Period& swapObsLag,
        const Date& maturity,
        Calendar calendar,
        BusinessDayConvention paymentConvention,
        DayCounter dayCounter,
        ext::shared_ptr<YoYInflationIndex> yii,
        Handle<YieldTermStructure> nominalTermStructure)
    : BootstrapHelper<YoYInflationTermStructure>(quote), swapObsLag_(swapObsLag),
      maturity_(maturity), calendar_(std::move(calendar)), paymentConvention_(paymentConvention),
      dayCounter_(std::move(dayCounter)), yii_(std::move(yii)),
      nominalTermStructure_(std::move(nominalTermStructure)) {

        if (yii_->interpolated()) {
            // if interpolated then simple
            earliestDate_ = maturity_ - swapObsLag_;
            latestDate_ = maturity_ - swapObsLag_;
        } else {
            // but if NOT interpolated then the value is valid
            // for every day in an inflation period so you actually
            // get an extended validity, however for curve building
            // just put the first date because using that convention
            // for the base date throughout
            std::pair<Date, Date> limStart =
                inflationPeriod(maturity_ - swapObsLag_, yii_->frequency());
            earliestDate_ = limStart.first;
            latestDate_ = limStart.first;
        }

        // check that the observation lag of the swap
        // is compatible with the availability lag of the index AND
        // it's interpolation (assuming the start day is spot)
        if (yii_->interpolated()) {
            Period pShift(yii_->frequency());
            QL_REQUIRE(swapObsLag_ - pShift >= yii_->availabilityLag(),
                       "inconsistency between swap observation lag "
                           << swapObsLag_ << ", index period " << pShift << " and index availability "
                           << yii_->availabilityLag() << ": need (obsLag-index period) >= availLag");
        }

        registerWith(Settings::instance().evaluationDate());
        registerWith(nominalTermStructure_);
    }


    Real YearOnYearInflationSwapHelper::impliedQuote() const {
        yyiis_->deepUpdate();
        return yyiis_->fairRate();
    }


    void YearOnYearInflationSwapHelper::setTermStructure(YoYInflationTermStructure* y) {

        BootstrapHelper<YoYInflationTermStructure>::setTermStructure(y);

        // set up a new YYIIS
        // but this one does NOT own its inflation term structure
        const bool own = false;

        // The effect of the new inflation term structure is
        // felt via the effect on the inflation index
        Handle<YoYInflationTermStructure> yyts(
            ext::shared_ptr<YoYInflationTermStructure>(y, null_deleter()), own);

        ext::shared_ptr<YoYInflationIndex> new_yii = yii_->clone(yyts);

        // always works because tenor is always 1 year so
        // no problem with different days-in-month
        Date from = Settings::instance().evaluationDate();
        Date to = maturity_;
        Schedule fixedSchedule = MakeSchedule()
                                     .from(from)
                                     .to(to)
                                     .withTenor(1 * Years)
                                     .withConvention(Unadjusted)
                                     .withCalendar(calendar_) // fixed leg gets cal from sched
                                     .backwards();
        const Schedule& yoySchedule = fixedSchedule;
        Spread spread = 0.0;
        Rate fixedRate = quote()->value();

        Real nominal = 1000000.0; // has to be something but doesn't matter what
        yyiis_ = ext::make_shared<YearOnYearInflationSwap>(
            Swap::Payer, nominal, fixedSchedule, fixedRate, dayCounter_,
            yoySchedule, new_yii, swapObsLag_, spread, dayCounter_,
            calendar_, // inflation index does not have a calendar
            paymentConvention_);

        // The instrument takes a standard discounting swap engine.
        // The inflation-related work is done by the coupons.

        yyiis_->setPricingEngine(
            ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(nominalTermStructure_)));
    }

}
