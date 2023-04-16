/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009, 2011 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/instruments/cpiswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    // accrual adjustment is already in the schedules, as are calendars
    CPISwap::CPISwap(Type type,
                     Real nominal,
                     bool subtractInflationNominal,
                     // float + spread leg
                     Spread spread,
                     DayCounter floatDayCount,
                     Schedule floatSchedule,
                     const BusinessDayConvention& floatPaymentRoll,
                     Natural fixingDays,
                     std::shared_ptr<IborIndex> floatIndex,
                     // fixed x inflation leg
                     Rate fixedRate,
                     Real baseCPI,
                     DayCounter fixedDayCount,
                     Schedule fixedSchedule,
                     const BusinessDayConvention& fixedPaymentRoll,
                     const Period& observationLag,
                     std::shared_ptr<ZeroInflationIndex> fixedIndex,
                     CPI::InterpolationType observationInterpolation,
                     Real inflationNominal)
    : Swap(2), type_(type), nominal_(nominal), subtractInflationNominal_(subtractInflationNominal),
      spread_(spread), floatDayCount_(std::move(floatDayCount)),
      floatSchedule_(std::move(floatSchedule)), floatPaymentRoll_(floatPaymentRoll),
      fixingDays_(fixingDays), floatIndex_(std::move(floatIndex)), fixedRate_(fixedRate),
      baseCPI_(baseCPI), fixedDayCount_(std::move(fixedDayCount)),
      fixedSchedule_(std::move(fixedSchedule)), fixedPaymentRoll_(fixedPaymentRoll),
      fixedIndex_(std::move(fixedIndex)), observationLag_(observationLag),
      observationInterpolation_(observationInterpolation) {
        QL_REQUIRE(!floatSchedule_.empty(), "empty float schedule");
        QL_REQUIRE(!fixedSchedule_.empty(), "empty fixed schedule");
        // \todo if roll!=unadjusted then need calendars ...

        if (inflationNominal==Null<Real>()) inflationNominal_ = nominal_;
        else inflationNominal_ = inflationNominal;

        Leg floatingLeg;
        if (floatSchedule_.size() > 1) {
            floatingLeg = IborLeg(floatSchedule_, floatIndex_)
            .withNotionals(nominal_)
            .withSpreads(spread_)
            .withPaymentDayCounter(floatDayCount_)
            .withPaymentAdjustment(floatPaymentRoll_)
            .withFixingDays(fixingDays_);
        }

        if (floatSchedule_.size()==1 ||
            !subtractInflationNominal_ ||
            (subtractInflationNominal && std::fabs(nominal_-inflationNominal_)>0.00001)
            )
        {
            Date payNotional;
            if (floatSchedule_.size()==1) { // no coupons
                payNotional = floatSchedule_[0];
                payNotional = floatSchedule_.calendar().adjust(payNotional, floatPaymentRoll_);
            } else { // use the pay date of the last coupon
                payNotional = floatingLeg.back()->date();
            }

            Real floatAmount = subtractInflationNominal_ ? nominal_ - inflationNominal_ : nominal_;
            std::shared_ptr<CashFlow> nf(new SimpleCashFlow(floatAmount, payNotional));
            floatingLeg.push_back(nf);
        }

        // a CPIleg know about zero legs and inclusion of base inflation notional
        Leg cpiLeg = CPILeg(fixedSchedule_, fixedIndex_,
                            baseCPI_, observationLag_)
        .withNotionals(inflationNominal_)
        .withFixedRates(fixedRate_)
        .withPaymentDayCounter(fixedDayCount_)
        .withPaymentAdjustment(fixedPaymentRoll_)
        .withObservationInterpolation(observationInterpolation_)
        .withSubtractInflationNominal(subtractInflationNominal_);


        Leg::const_iterator i;
        for (i = cpiLeg.begin(); i < cpiLeg.end(); ++i) {
            registerWith(*i);
        }

        for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i) {
            registerWith(*i);
        }

        legs_[0] = cpiLeg;
        legs_[1] = floatingLeg;

        if (type_==Payer) {
            payer_[0] = 1.0;
            payer_[1] = -1.0;
        } else {
            payer_[0] = -1.0;
            payer_[1] = 1.0;
        }
    }


    //! for simple case sufficient to copy base class
    void CPISwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<CPISwap::arguments*>(args);

        if (arguments == nullptr)
            return; // it's a swap engine...
    }


    Rate CPISwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Rate>(), "result not available");
        return fairRate_;
    }

    Spread CPISwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "result not available");
        return fairSpread_;
    }


    Real CPISwap::fixedLegNPV() const {//FIXME
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real CPISwap::floatLegNPV() const {//FIXME
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    void CPISwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    void CPISwap::fetchResults(const PricingEngine::results* r) const {
        static const Spread basisPoint = 1.0e-4;

        // copy from VanillaSwap
        // works because similarly simple instrument
        // that we always expect to be priced with a swap engine

        Swap::fetchResults(r);

        const auto* results = dynamic_cast<const CPISwap::results*>(r);
        if (results != nullptr) { // might be a swap engine, so no error is thrown
            fairRate_ = results->fairRate;
            fairSpread_ = results->fairSpread;
        } else {
            fairRate_ = Null<Rate>();
            fairSpread_ = Null<Spread>();
        }

        if (fairRate_ == Null<Rate>()) {
            // calculate it from other results
            if (legBPS_[0] != Null<Real>())
                fairRate_ = fixedRate_ - NPV_/(legBPS_[0]/basisPoint);
        }
        if (fairSpread_ == Null<Spread>()) {
            // ditto
            if (legBPS_[1] != Null<Real>())
                fairSpread_ = spread_ - NPV_/(legBPS_[1]/basisPoint);
        }

    }

    void CPISwap::arguments::validate() const {
        Swap::arguments::validate();
    }

    void CPISwap::results::reset() {
        Swap::results::reset();
        fairRate = Null<Rate>();
        fairSpread = Null<Spread>();
    }

}

