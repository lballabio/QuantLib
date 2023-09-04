/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <utility>

namespace QuantLib {

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Spread spread,
                                               Natural paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           schedule,
                           fixedRate,
                           std::move(fixedDC),
                           std::move(overnightIndex),
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               const std::vector<Real>& nominals,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Spread spread,
                                               Natural paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : OvernightIndexedSwap(type,
                           nominals,
                           schedule,
                           fixedRate,
                           std::move(fixedDC),
                           nominals,
                           schedule,
                           std::move(overnightIndex),
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               const Schedule& fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const Schedule& overnightSchedule,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Spread spread,
                                               Natural paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           fixedSchedule,
                           fixedRate,
                           std::move(fixedDC),
                           std::vector<Real>(1, nominal),
                           overnightSchedule,
                           std::move(overnightIndex),
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               std::vector<Real> fixedNominals,
                                               Schedule fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               std::vector<Real> overnightNominals,
                                               Schedule overnightSchedule,
                                               ext::shared_ptr<OvernightIndex> overnightIndex,
                                               Spread spread,
                                               Natural paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod)
    : Swap(2), type_(type), fixedNominals_(std::move(fixedNominals)),
      fixedSchedule_(std::move(fixedSchedule)), fixedRate_(fixedRate), fixedDC_(std::move(fixedDC)),
      overnightNominals_(std::move(overnightNominals)), overnightSchedule_(std::move(overnightSchedule)),
      overnightIndex_(std::move(overnightIndex)), spread_(spread), averagingMethod_(averagingMethod) {
        if (fixedDC_ == DayCounter())
            fixedDC_ = overnightIndex_->dayCounter();
        legs_[0] = FixedRateLeg(fixedSchedule_)
                       .withNotionals(fixedNominals_)
                       .withCouponRates(fixedRate_, fixedDC_)
                       .withPaymentLag(paymentLag)
                       .withPaymentAdjustment(paymentAdjustment)
                       .withPaymentCalendar(paymentCalendar.empty() ? fixedSchedule_.calendar() :
                                                                      paymentCalendar);

        legs_[1] =
            OvernightLeg(overnightSchedule_, overnightIndex_)
                .withNotionals(overnightNominals_)
                .withSpreads(spread_)
                .withTelescopicValueDates(telescopicValueDates)
                .withPaymentLag(paymentLag)
                .withPaymentAdjustment(paymentAdjustment)
                .withPaymentCalendar(paymentCalendar.empty() ? overnightSchedule_.calendar() :
                                                               paymentCalendar)
                .withAveragingMethod(averagingMethod_);

        for (Size j = 0; j < 2; ++j) {
            for (auto& i : legs_[j])
                registerWith(i);
        }

        switch (type_) {
            case Payer:
                payer_[0] = -1.0;
                payer_[1] = +1.0;
                break;
            case Receiver:
                payer_[0] = +1.0;
                payer_[1] = -1.0;
                break;
            default:
                QL_FAIL("Unknown overnight-swap type");
        }

        // These bools tell us if we can support the old methods nominal() and nominals().
        // There might be false negatives (i.e., if we pass constant vectors of different lengths
        // as fixedNominals and floatingNominals) but we're going to assume that whoever uses the
        // constructor with two vectors is mostly going to use the new methods instead.
        sameNominals_ = std::equal(fixedNominals_.begin(), fixedNominals_.end(),
                                   overnightNominals_.begin(), overnightNominals_.end());
        if (!sameNominals_) {
            constantNominals_ = false;
        } else {
            constantNominals_ = true;
            Real front = fixedNominals_[0];
            for (auto x : fixedNominals_) {
                if (x != front) {
                    constantNominals_ = false;
                    break;
                }
            }
        }
    }

    Real OvernightIndexedSwap::fairRate() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return fixedRate_ - NPV_ / (fixedLegBPS() / basisPoint);
    }

    Spread OvernightIndexedSwap::fairSpread() const {
        static Spread basisPoint = 1.0e-4;
        calculate();
        return spread_ - NPV_ / (overnightLegBPS() / basisPoint);
    }

    Real OvernightIndexedSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real OvernightIndexedSwap::overnightLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real OvernightIndexedSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real OvernightIndexedSwap::overnightLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    Real OvernightIndexedSwap::nominal() const {
        QL_REQUIRE(constantNominals_, "varying nominals");
        return fixedNominals_[0];
    }

    const std::vector<Real>& OvernightIndexedSwap::nominals() const {
        QL_REQUIRE(sameNominals_, "different nominals on fixed and floating leg");
        return fixedNominals_;
    }

}
