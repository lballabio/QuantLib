/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 Copyright (C) 2025 Paolo D'Elia
 All rights reserved.

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

#include <ql/instruments/constnotionalcrossccyfixfloatswap.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>

namespace QuantLib {

ConstNotionalCrossCcyFixFloatSwap::ConstNotionalCrossCcyFixFloatSwap(
    Type type, Real fixedNominal, const Currency& fixedCurrency, const Schedule& fixedSchedule, Rate fixedRate,
    const DayCounter& fixedDayCount, BusinessDayConvention fixedPaymentBdc, Natural fixedPaymentLag,
    const Calendar& fixedPaymentCalendar, Real floatNominal, const Currency& floatCurrency,
    const Schedule& floatSchedule, const ext::shared_ptr<IborIndex>& floatIndex, Spread floatSpread,
    BusinessDayConvention floatPaymentBdc, Natural floatPaymentLag, const Calendar& floatPaymentCalendar,
    const bool telescopicValueDates, ext::optional<bool> floatIncludeSpread, ext::optional<Natural> floatLookbackDays,
    ext::optional<Size> floatLockoutDays, ext::optional<bool> floatIsAveraged)
    : ConstNotionalCrossCcySwap(2), type_(type), fixedNominal_(fixedNominal), fixedCurrency_(fixedCurrency),
      fixedSchedule_(fixedSchedule), fixedRate_(fixedRate), fixedDayCount_(fixedDayCount),
      fixedPaymentBdc_(fixedPaymentBdc), fixedPaymentLag_(fixedPaymentLag), fixedPaymentCalendar_(fixedPaymentCalendar),
      floatNominal_(floatNominal), floatCurrency_(floatCurrency), floatSchedule_(floatSchedule),
      floatIndex_(floatIndex), floatSpread_(floatSpread), floatPaymentBdc_(floatPaymentBdc),
      floatPaymentLag_(floatPaymentLag), floatPaymentCalendar_(floatPaymentCalendar),
      telescopicValueDates_(telescopicValueDates),
      floatIncludeSpread_(floatIncludeSpread), floatLookbackDays_(floatLookbackDays),
      floatLockoutDays_(floatLockoutDays), floatIsAveraged_(floatIsAveraged) {

    // Build the float leg
    Leg floatLeg;
    if (auto on = ext::dynamic_pointer_cast<OvernightIndex>(floatIndex_)) {
        floatLeg = OvernightLeg(floatSchedule_, on)
                    .withNotionals(floatNominal_)
                    .withSpreads(floatSpread_)
                    .withPaymentAdjustment(floatPaymentBdc_)
                    .withPaymentLag(floatPaymentLag_)
                    .withLookbackDays(floatLookbackDays ? *floatLookbackDays : 0)
                    .withPaymentCalendar(floatPaymentCalendar_)
                    .withLockoutDays(floatLockoutDays_ ? *floatLockoutDays_ : 0)
                    .withAveragingMethod(floatIsAveraged_ ? 
                        (*floatIsAveraged_ ? RateAveraging::Simple : RateAveraging::Compound) : RateAveraging::Compound)
                    .withTelescopicValueDates(telescopicValueDates_);
    } else {
        floatLeg = IborLeg(floatSchedule_, floatIndex_)
                       .withNotionals(floatNominal_)
                       .withSpreads(floatSpread_)
                       .withPaymentAdjustment(floatPaymentBdc_)
                       .withPaymentLag(floatPaymentLag_)
                       .withPaymentCalendar(floatPaymentCalendar_);
    }

    // Register with each floating rate coupon
    for (Leg::const_iterator it = floatLeg.begin(); it < floatLeg.end(); ++it)
        registerWith(*it);

    // Build the fixed rate leg
    Leg fixedLeg = FixedRateLeg(fixedSchedule_)
                       .withNotionals(fixedNominal_)
                       .withCouponRates(fixedRate_, fixedDayCount_)
                       .withPaymentAdjustment(fixedPaymentBdc_)
                       .withPaymentLag(fixedPaymentLag)
                       .withPaymentCalendar(fixedPaymentCalendar);

    auto earliestDate = std::min(CashFlows::startDate(floatLeg),
                                 CashFlows::startDate(fixedLeg));

    auto maturityDate = std::max(CashFlows::maturityDate(floatLeg),
                                 CashFlows::maturityDate(fixedLeg));

    // Add notional exchanges on float Leg
    ConstNotionalCrossCcySwap::addNotionalExchangesToLeg(floatLeg, floatPaymentCalendar_, earliestDate, maturityDate,
                                            floatPaymentLag_, floatPaymentBdc_, floatNominal_);
    
    // Add notional exchanges on fixed leg
    ConstNotionalCrossCcySwap::addNotionalExchangesToLeg(fixedLeg, fixedPaymentCalendar_, earliestDate, maturityDate,
                                            fixedPaymentLag_, fixedPaymentBdc_, fixedNominal_);

    // Deriving from cross currency swap where:
    //   First leg should hold the pay flows
    //   Second leg should hold the receive flows
    payer_[0] = -1.0;
    payer_[1] = 1.0;
    switch (type_) {
    case Payer:
        legs_[0] = fixedLeg;
        currencies_[0] = fixedCurrency_;
        legs_[1] = floatLeg;
        currencies_[1] = floatCurrency_;
        break;
    case Receiver:
        legs_[1] = fixedLeg;
        currencies_[1] = fixedCurrency_;
        legs_[0] = floatLeg;
        currencies_[0] = floatCurrency_;
        break;
    default:
        QL_FAIL("Unknown cross currency fix float swap type");
    }
}

void ConstNotionalCrossCcyFixFloatSwap::setupArguments(PricingEngine::arguments* a) const {
    ConstNotionalCrossCcySwap::setupArguments(a);
    if (ConstNotionalCrossCcyFixFloatSwap::arguments* args = dynamic_cast<ConstNotionalCrossCcyFixFloatSwap::arguments*>(a)) {
        args->fixedRate = fixedRate_;
        args->spread = floatSpread_;
    }
}

void ConstNotionalCrossCcyFixFloatSwap::fetchResults(const PricingEngine::results* r) const {

    ConstNotionalCrossCcySwap::fetchResults(r);

    // Depending on the pricing engine used, we may have ConstNotionalCrossCcyFixFloatSwap::results
    if (const ConstNotionalCrossCcyFixFloatSwap::results* res = dynamic_cast<const ConstNotionalCrossCcyFixFloatSwap::results*>(r)) {
        // If we have ConstNotionalCrossCcyFixFloatSwap::results from the pricing engine
        fairFixedRate_ = res->fairFixedRate;
        fairSpread_ = res->fairSpread;
    } else {
        // If not, set them to Null to indicate a calculation is needed below
        fairFixedRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    // Calculate fair rate and spread if they are still Null here
    static Spread basisPoint = 1.0e-4;

    Size idxFixed = type_ == Payer ? 0 : 1;
    if (fairFixedRate_ == Null<Rate>() && legBPS_[idxFixed] != Null<Real>())
        fairFixedRate_ = fixedRate_ - NPV_ / (legBPS_[idxFixed] / basisPoint);

    Size idxFloat = type_ == Payer ? 1 : 0;
    if (fairSpread_ == Null<Spread>() && legBPS_[idxFloat] != Null<Real>())
        fairSpread_ = floatSpread_ - NPV_ / (legBPS_[idxFloat] / basisPoint);
}

void ConstNotionalCrossCcyFixFloatSwap::setupExpired() const {
    ConstNotionalCrossCcySwap::setupExpired();
    fairFixedRate_ = Null<Rate>();
    fairSpread_ = Null<Spread>();
}

void ConstNotionalCrossCcyFixFloatSwap::arguments::validate() const {
    ConstNotionalCrossCcySwap::arguments::validate();
    QL_REQUIRE(fixedRate != Null<Rate>(), "Fixed rate cannot be null");
    QL_REQUIRE(spread != Null<Spread>(), "Spread cannot be null");
}

void ConstNotionalCrossCcyFixFloatSwap::results::reset() {
    ConstNotionalCrossCcySwap::results::reset();
    fairFixedRate = Null<Rate>();
    fairSpread = Null<Spread>();
}

} // namespace QuantLib
