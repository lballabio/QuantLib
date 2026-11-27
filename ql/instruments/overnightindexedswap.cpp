/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer
 Copyright (C) 2026 Sergio Araujo
 Copyright (C) 2026 Thiago Rodrigues

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision,
                                               Compounding fixedCompounding,
                                               Frequency fixedCompoundingFrequency)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           schedule,
                           fixedRate,
                           std::move(fixedDC),
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod,
                           lookbackDays,
                           lockoutDays,
                           applyObservationShift,
                           roundingPrecision,
                           fixedCompounding,
                           fixedCompoundingFrequency) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               const std::vector<Real>& nominals,
                                               const Schedule& schedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision,
                                               Compounding fixedCompounding,
                                               Frequency fixedCompoundingFrequency)
    : OvernightIndexedSwap(type,
                           nominals,
                           schedule,
                           fixedRate,
                           std::move(fixedDC),
                           nominals,
                           schedule,
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod,
                           lookbackDays,
                           lockoutDays,
                           applyObservationShift,
                           roundingPrecision,
                           fixedCompounding,
                           fixedCompoundingFrequency) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               Real nominal,
                                               Schedule fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               Schedule overnightSchedule,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision,
                                               Compounding fixedCompounding,
                                               Frequency fixedCompoundingFrequency)
    : OvernightIndexedSwap(type,
                           std::vector<Real>(1, nominal),
                           std::move(fixedSchedule),
                           fixedRate,
                           std::move(fixedDC),
                           std::vector<Real>(1, nominal),
                           std::move(overnightSchedule),
                           overnightIndex,
                           spread,
                           paymentLag,
                           paymentAdjustment,
                           paymentCalendar,
                           telescopicValueDates,
                           averagingMethod,
                           lookbackDays,
                           lockoutDays,
                           applyObservationShift,
                           roundingPrecision,
                           fixedCompounding,
                           fixedCompoundingFrequency) {}

    OvernightIndexedSwap::OvernightIndexedSwap(Type type,
                                               std::vector<Real> fixedNominals,
                                               Schedule fixedSchedule,
                                               Rate fixedRate,
                                               DayCounter fixedDC,
                                               const std::vector<Real>& overnightNominals,
                                               Schedule overnightSchedule,
                                               const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                               Spread spread,
                                               Integer paymentLag,
                                               BusinessDayConvention paymentAdjustment,
                                               const Calendar& paymentCalendar,
                                               bool telescopicValueDates,
                                               RateAveraging::Type averagingMethod,
                                               Natural lookbackDays,
                                               Natural lockoutDays,
                                               bool applyObservationShift,
                                               const std::optional<Integer>& roundingPrecision,
                                               Compounding fixedCompounding,
                                               Frequency fixedCompoundingFrequency)
    : FixedVsFloatingSwap(type, std::move(fixedNominals), std::move(fixedSchedule), fixedRate, std::move(fixedDC),
                          overnightNominals, std::move(overnightSchedule), overnightIndex,
                          spread, DayCounter(), paymentAdjustment, paymentLag, paymentCalendar),
                          overnightIndex_(overnightIndex),
                          paymentLag_(paymentLag), paymentCalendar_(paymentCalendar),
                          telescopicValueDates_(telescopicValueDates),
                          averagingMethod_(averagingMethod),
                          lookbackDays_(lookbackDays), lockoutDays_(lockoutDays),
                          applyObservationShift_(applyObservationShift),
                          roundingPrecision_(roundingPrecision),
                          fixedCompounding_(fixedCompounding),
                          fixedCompoundingFrequency_(fixedCompoundingFrequency) {
        if (fixedCompounding_ != Simple) {
            // the base class builds a fixed leg accruing simple interest;
            // this-> selects the accessors, not the moved-from arguments
            legs_[0] = FixedRateLeg(this->fixedSchedule())
                           .withNotionals(this->fixedNominals())
                           .withCouponRates(this->fixedRate(), fixedDayCount(), fixedCompounding_,
                                            fixedCompoundingFrequency_)
                           .withPaymentAdjustment(paymentConvention())
                           .withPaymentLag(paymentLag)
                           .withPaymentCalendar(paymentCalendar.empty() ?
                                                    this->fixedSchedule().calendar() :
                                                    paymentCalendar);
        }

        OvernightLeg leg(floatingSchedule(), overnightIndex_);
        leg
                .withNotionals(overnightNominals)
                .withSpreads(spread)
                .withTelescopicValueDates(telescopicValueDates)
                .withPaymentLag(paymentLag)
                .withPaymentAdjustment(paymentAdjustment)
                .withPaymentCalendar(paymentCalendar.empty() ?
                                     floatingSchedule().calendar() :
                                     paymentCalendar)
                .withAveragingMethod(averagingMethod_)
                .withLookbackDays(lookbackDays_)
                .withLockoutDays(lockoutDays_)
                .withObservationShift(applyObservationShift_);
        if (roundingPrecision_.has_value()) {
            QL_REQUIRE(*roundingPrecision_ >= 0 && *roundingPrecision_ <= 16,
                       "rounding precision (" << *roundingPrecision_ <<
                       ") must be between 0 and 16");
            leg.withRoundingPrecision(*roundingPrecision_);
        }
        legs_[1] = leg;
        for (const auto& c : legs_[1])
            registerWith(c);
    }

    void OvernightIndexedSwap::setupFloatingArguments(arguments* args) const {
        const Leg& floatingCoupons = floatingLeg();
        Size n = floatingCoupons.size();

        args->floatingResetDates = args->floatingPayDates = args->floatingFixingDates = std::vector<Date>(n);
        args->floatingAccrualTimes = std::vector<Time>(n);
        args->floatingSpreads = std::vector<Spread>(n);
        args->floatingCoupons = args->floatingNominals = std::vector<Real>(n);

        for (Size i=0; i<n; ++i) {
            auto coupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(floatingCoupons[i]);

            args->floatingResetDates[i] = coupon->accrualStartDate();
            args->floatingPayDates[i] = coupon->date();
            args->floatingNominals[i] = coupon->nominal();

            args->floatingFixingDates[i] = coupon->fixingDate();
            args->floatingAccrualTimes[i] = coupon->accrualPeriod();
            args->floatingSpreads[i] = coupon->spread();
            try {
                args->floatingCoupons[i] = coupon->amount();
            } catch (Error&) {
                args->floatingCoupons[i] = Null<Real>();
            }
        }
    }

    void OvernightIndexedSwap::fetchResults(const PricingEngine::results* r) const {
        FixedVsFloatingSwap::fetchResults(r);

        if (fixedCompounding_ != Simple) {
            // the fixed-leg NPV is not linear in the rate; solve for it instead
            fairRate_ = Null<Rate>();
            if (NPV_ != Null<Real>() && legNPV_[0] != Null<Real>()) {
                try {
                    fairRate_ = fixedRateForNPV(legNPV_[0] - NPV_);
                } catch (Error&) {
                    fairRate_ = Null<Rate>();
                }
            }
        }
    }

    Rate OvernightIndexedSwap::impliedFixedRate(Real fixedLegNPV) const {
        calculate();
        return fixedRateForNPV(fixedLegNPV);
    }

    Rate OvernightIndexedSwap::fixedRateForNPV(Real fixedLegNPV) const {
        static const Spread basisPoint = 1.0e-4;

        QL_REQUIRE(legNPV_[0] != Null<Real>() && legBPS_[0] != Null<Real>(),
                   "fixed-leg NPV and BPS not available");
        QL_REQUIRE(legBPS_[0] != 0.0, "null fixed-leg BPS");

        // exact for simple interest, a starting guess otherwise
        Rate linearRate = fixedRate() + (fixedLegNPV - legNPV_[0]) / (legBPS_[0] / basisPoint);
        if (fixedCompounding_ == Simple)
            return linearRate;

        const Leg& leg = fixedLeg();

        if (leg.size() == 1) {
            // single coupon: c(K) follows from NPV and BPS, no curve needed
            auto coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(leg[0]);
            Real compound = 1.0 + fixedLegNPV * coupon->accrualPeriod() * basisPoint / legBPS_[0];
            QL_REQUIRE(compound > 0.0, "non-positive compound factor ("
                                           << compound << ") implied by fixed-leg NPV ("
                                           << fixedLegNPV << ")");
            const InterestRate& r = coupon->interestRate();
            return InterestRate::impliedRate(
                       compound, r.dayCounter(), r.compounding(), r.frequency(),
                       coupon->accrualStartDate(), coupon->accrualEndDate(),
                       coupon->referencePeriodStart(), coupon->referencePeriodEnd())
                .rate();
        }

        // several coupons: we need the discount curve
        auto engine = ext::dynamic_pointer_cast<DiscountingSwapEngine>(engine_);
        QL_REQUIRE(engine != nullptr,
                   "a DiscountingSwapEngine is required to calculate the fixed rate "
                   "of a fixed leg with several compounded coupons");
        const Handle<YieldTermStructure>& discountCurve = engine->discountCurve();
        QL_REQUIRE(!discountCurve.empty(), "discounting term structure handle is empty");
        QL_REQUIRE(valuationDate_ != Date(), "valuation date not available");

        // use the same cash flows and NPV date as the engine
        bool includeSettlementDateFlows =
            engine->includeSettlementDateFlows() ? // NOLINT(readability-implicit-bool-conversion)
                *engine->includeSettlementDateFlows() :
                Settings::instance().includeReferenceDateEvents();
        Date settlementDate = engine->settlementDate() == Date() ?
                                  discountCurve->referenceDate() :
                                  engine->settlementDate();

        // the CashFlows function works with the NPV of the leg, without the payer sign
        return CashFlows::impliedFixedRate(leg, **discountCurve, includeSettlementDateFlows,
                                           settlementDate, valuationDate_,
                                           payer_[0] * fixedLegNPV, 1.0e-12, 100);
    }

}
