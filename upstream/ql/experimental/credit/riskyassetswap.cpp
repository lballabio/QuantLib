/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Roland Lichters

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

#include <ql/event.hpp>
#include <ql/experimental/credit/riskyassetswap.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    RiskyAssetSwap::RiskyAssetSwap(bool fixedPayer,
                                   Real nominal,
                                   Schedule fixedSchedule,
                                   Schedule floatSchedule,
                                   DayCounter fixedDayCounter,
                                   DayCounter floatDayCounter,
                                   Rate spread,
                                   Rate recoveryRate,
                                   Handle<YieldTermStructure> yieldTS,
                                   Handle<DefaultProbabilityTermStructure> defaultTS,
                                   Rate coupon)
    : fixedPayer_(fixedPayer), nominal_(nominal), fixedSchedule_(std::move(fixedSchedule)),
      floatSchedule_(std::move(floatSchedule)), fixedDayCounter_(std::move(fixedDayCounter)),
      floatDayCounter_(std::move(floatDayCounter)), spread_(spread), recoveryRate_(recoveryRate),
      yieldTS_(std::move(yieldTS)), defaultTS_(std::move(defaultTS)), coupon_(coupon) {

        registerWith (yieldTS_);
        registerWith (defaultTS_);
    }

    bool RiskyAssetSwap::isExpired () const {
        return detail::simple_event(fixedSchedule_.dates().back())
               .hasOccurred(yieldTS_->referenceDate());
    }


    void RiskyAssetSwap::setupExpired() const {
        Instrument::setupExpired();
    }


    void RiskyAssetSwap::performCalculations() const {
        // order of calls is essential
        floatAnnuity_   = floatAnnuity();
        fixedAnnuity_   = fixedAnnuity();
        parCoupon_      = parCoupon();

        if (coupon_ == Null<Rate>())  coupon_ = parCoupon_;

        recoveryValue_  = recoveryValue();
        riskyBondPrice_ = riskyBondPrice();

        NPV_ = riskyBondPrice_
            - coupon_ * fixedAnnuity_
            + yieldTS_->discount (fixedSchedule_.dates().front())
            - yieldTS_->discount (fixedSchedule_.dates().back())
            + spread_ * floatAnnuity_;

        NPV_ *= nominal_;

        if (!fixedPayer_)
            NPV_ *= -1;
    }


    Real RiskyAssetSwap::floatAnnuity () const {
        Real annuity = 0;
        for (Size i = 1; i < floatSchedule_.size(); i++) {
            Time dcf = floatDayCounter_.yearFraction (floatSchedule_[i-1],
                                                      floatSchedule_[i]);
            annuity += dcf * yieldTS_->discount (floatSchedule_[i]);
        }
        return annuity;
    }


    Real RiskyAssetSwap::fixedAnnuity () const {
        Real annuity = 0;
        for (Size i = 1; i < floatSchedule_.size(); i++) {
            Time dcf = fixedDayCounter_.yearFraction (floatSchedule_[i-1],
                                                      floatSchedule_[i]);
            annuity += dcf * yieldTS_->discount (floatSchedule_[i]);
        }
        return annuity;
    }


    Real RiskyAssetSwap::parCoupon () const {
        return (yieldTS_->discount(fixedSchedule_.dates().front())
                -yieldTS_->discount(fixedSchedule_.dates().back()))
            / fixedAnnuity_;
    }


    Real RiskyAssetSwap::recoveryValue() const {
        Real recoveryValue = 0;
        // simple Euler integral to evaluate the recovery value
        for (Size i = 1; i < fixedSchedule_.size(); i++) {
            TimeUnit stepSize = Days;
            Date d;
            if (fixedSchedule_[i-1] >= defaultTS_->referenceDate())
                d = fixedSchedule_[i-1];
            else
                d = defaultTS_->referenceDate();
            Date d0 = d;
            do {
                Real disc = yieldTS_->discount (d);
                Real dd   = defaultTS_->defaultDensity (d, true);
                Real dcf  = defaultTS_->dayCounter().yearFraction (d0, d);

                recoveryValue  += disc * dd * dcf;

                d0 = d;

                d = NullCalendar().advance (d0, 1, stepSize, Unadjusted);
            }
            while (d < fixedSchedule_[i]);
        }
        recoveryValue *= recoveryRate_;

        return recoveryValue;
    }


    Real RiskyAssetSwap::riskyBondPrice () const {
        Real value = 0;
        for (Size i = 1; i < fixedSchedule_.size(); i++) {
            Time dcf = fixedDayCounter_.yearFraction (fixedSchedule_[i-1],
                                                      fixedSchedule_[i]);
            value += dcf * yieldTS_->discount (fixedSchedule_[i])
                * defaultTS_->survivalProbability (fixedSchedule_[i], true);
        }
        value *= coupon_;

        value += yieldTS_->discount (fixedSchedule_.dates().back())
            * defaultTS_->survivalProbability (fixedSchedule_.dates().back(),
                                               true);

        return value + recoveryValue_;
    }


    Real RiskyAssetSwap::fairSpread () {
        calculate();

        Real value = 0;
        for (Size i = 1; i < fixedSchedule_.size(); i++) {
            Time dcf = fixedDayCounter_.yearFraction (fixedSchedule_[i-1],
                                                      fixedSchedule_[i]);
            value += dcf * yieldTS_->discount (fixedSchedule_[i])
                * defaultTS_->defaultProbability (fixedSchedule_[i], true);
        }
        value *= coupon_;

        value += yieldTS_->discount (fixedSchedule_.dates().back())
            * defaultTS_->defaultProbability (fixedSchedule_.dates().back(),
                                              true);

        Real initialDiscount = yieldTS_->discount(fixedSchedule_[0]);

        return (1.0 - initialDiscount + value - recoveryValue_) / fixedAnnuity_;
    }


    AssetSwapHelper::AssetSwapHelper(const Handle<Quote>& spread,
                                     const Period& tenor,
                                     Natural settlementDays,
                                     Calendar calendar,
                                     const Period& fixedPeriod,
                                     BusinessDayConvention fixedConvention,
                                     DayCounter fixedDayCount,
                                     const Period& floatPeriod,
                                     BusinessDayConvention floatConvention,
                                     DayCounter floatDayCount,
                                     Real recoveryRate,
                                     const RelinkableHandle<YieldTermStructure>& yieldTS,
                                     const Period& integrationStepSize)
    : DefaultProbabilityHelper(spread), tenor_(tenor), settlementDays_(settlementDays),
      calendar_(std::move(calendar)), fixedConvention_(fixedConvention), fixedPeriod_(fixedPeriod),
      fixedDayCount_(std::move(fixedDayCount)), floatConvention_(floatConvention),
      floatPeriod_(floatPeriod), floatDayCount_(std::move(floatDayCount)),
      recoveryRate_(recoveryRate), yieldTS_(yieldTS), integrationStepSize_(integrationStepSize) {

        initializeDates();

        registerWith(Settings::instance().evaluationDate());
        registerWith(yieldTS);
    }

    Real AssetSwapHelper::impliedQuote() const {
        QL_REQUIRE(!probability_.empty(),
                   "default term structure not set");
        // we didn't register as observers - force calculation
        asw_->recalculate();
        return asw_->fairSpread();
    }

    void AssetSwapHelper::setTermStructure(
                                        DefaultProbabilityTermStructure* ts) {
        DefaultProbabilityHelper::setTermStructure(ts);

        probability_.linkTo(
            ext::shared_ptr<DefaultProbabilityTermStructure>(ts, null_deleter()),
            false);

        initializeDates();
    }

    void AssetSwapHelper::update() {
        if (evaluationDate_ != Settings::instance().evaluationDate())
            initializeDates();

        DefaultProbabilityHelper::update();
    }

    void AssetSwapHelper::initializeDates() {
        evaluationDate_ = Settings::instance().evaluationDate();

        earliestDate_ = calendar_.advance (evaluationDate_,
                                           settlementDays_, Days);

        Date maturity = earliestDate_ + tenor_;

        latestDate_ = calendar_.adjust (maturity, fixedConvention_);

        Schedule fixedSchedule(earliestDate_, maturity,
                               fixedPeriod_, calendar_,
                               fixedConvention_, fixedConvention_,
                               DateGeneration::Forward, false);
        Schedule floatSchedule(earliestDate_, maturity,
                               floatPeriod_, calendar_,
                               floatConvention_, floatConvention_,
                               DateGeneration::Forward, false);

        asw_ = ext::make_shared<RiskyAssetSwap>(true,
                                                         100.0,
                                                         fixedSchedule,
                                                         floatSchedule,
                                                         fixedDayCount_,
                                                         floatDayCount_,
                                                         0.01,
                                                         recoveryRate_,
                                                         yieldTS_,
                                                         probability_);
    }

}
