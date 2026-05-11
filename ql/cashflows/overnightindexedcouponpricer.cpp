/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2016 Stefano Fondi
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

#include <ql/cashflows/overnightindexedcouponpricer.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        Size determineNumberOfFixings(const std::vector<Date>& interestDates,
                                      const Date& date) {
            return std::lower_bound(interestDates.begin(), interestDates.end()-1, date) -
                     interestDates.begin();
        }
    }

    OvernightIndexedCouponPricer::OvernightIndexedCouponPricer(
            Handle<OptionletVolatilityStructure> v,
            const bool effectiveVolatilityInput)
        : capletVol_(std::move(v)),
          effectiveVolatilityInput_(effectiveVolatilityInput) {
        registerWith(capletVol_);
    }

    void OvernightIndexedCouponPricer::initialize(const FloatingRateCoupon& coupon) {
        if (const auto *cfCoupon = dynamic_cast<const CappedFlooredOvernightIndexedCoupon*>(&coupon)) {
            auto *underlying = cfCoupon->underlying().get();
            QL_REQUIRE(underlying, "OvernightIndexedCouponPricer: CappedFlooredOvernightIndexedCoupon underlying coupon not defined");
            coupon_ = cfCoupon->underlying().get();
        } else if (const auto *onCoupon = dynamic_cast<const OvernightIndexedCoupon*>(&coupon)) {
            coupon_ = onCoupon;
        } else {
            QL_FAIL("OvernightIndexedCouponPricer: unsupported coupon type");
        }
    }

    bool OvernightIndexedCouponPricer::effectiveVolatilityInput() const {
        return effectiveVolatilityInput_;
    }

    Real OvernightIndexedCouponPricer::effectiveCapletVolatility() const {
        return effectiveCapletVolatility_;
    }

    Real OvernightIndexedCouponPricer::effectiveFloorletVolatility() const {
        return effectiveFloorletVolatility_;
    }
    
    CompoundingOvernightIndexedCouponPricer::CompoundingOvernightIndexedCouponPricer(
            Handle<OptionletVolatilityStructure> v,
            const bool effectiveVolatilityInput)
        : OvernightIndexedCouponPricer(std::move(v), effectiveVolatilityInput) {}

    Rate CompoundingOvernightIndexedCouponPricer::swapletRate() const {
        auto [swapletRate, effectiveSpread, effectiveIndexFixing] = compute(coupon_->accrualEndDate());
        swapletRate_ = swapletRate;
        effectiveSpread_ = effectiveSpread;
        effectiveIndexFixing_ = effectiveIndexFixing;
        return swapletRate;
    }

    Rate CompoundingOvernightIndexedCouponPricer::averageRate(const Date& date) const {
        auto [rate, effectiveSpread, effectiveIndexFixing] = compute(date);
        return rate;
    }

    Rate CompoundingOvernightIndexedCouponPricer::effectiveSpread() const {
        auto [r, effectiveSpread, effectiveIndexFixing] = compute(coupon_->accrualEndDate());
        effectiveSpread_ = effectiveSpread;
        return effectiveSpread_;
    }

    Rate CompoundingOvernightIndexedCouponPricer::effectiveIndexFixing() const {
        auto [r, effectiveSpread, effectiveIndexFixing] = compute(coupon_->accrualEndDate());
        effectiveIndexFixing_ = effectiveIndexFixing;
        return effectiveIndexFixing_;
    }

    std::tuple<Rate, Spread, Rate> CompoundingOvernightIndexedCouponPricer::compute(const Date& date) const {
	    const Date today = Settings::instance().evaluationDate();

        const ext::shared_ptr<OvernightIndex> index = ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());
        const auto& pastFixings = index->timeSeries();

        const auto& fixingDates = coupon_->fixingDates();
        const auto& valueDates = coupon_->valueDates();
        const auto& interestDates = coupon_->interestDates();
        const auto& dt = coupon_->dt();
        const bool applyObservationShift = coupon_->applyObservationShift() && coupon_->fixingDays() > 0;
        const auto& dc = index->dayCounter();
	    const Real couponSpread = coupon_->spread();
        const bool compoundSpreadDaily = coupon_->compoundSpreadDaily();

        Size i = 0;
        const Size n = determineNumberOfFixings(interestDates, date);
        const auto growthFactor = [&, applyObservationShift, compoundSpreadDaily](double fixing, Size idx) {
            const auto span = (applyObservationShift || date >= interestDates[idx + 1])
                              ? dt[idx]
                              : dc.yearFraction(interestDates[idx], date);
            const auto gf = 1.0 + fixing * span;
            return std::make_pair(gf, compoundSpreadDaily ? gf + couponSpread * span : gf);
        };

        Real compoundFactor = 1.0, compoundFactorWithoutSpread = 1.0;

        // already fixed part
        while (i < n && fixingDates[i] < today) {
            // rate must have been fixed
            Rate fixing = pastFixings[fixingDates[i]];
            QL_REQUIRE(fixing != Null<Real>(),
                       "Missing " << index->name() << " fixing for " << fixingDates[i]);
            const auto [gf, gfSpread] = growthFactor(fixing, i);
            compoundFactorWithoutSpread *= gf;
            compoundFactor *= gfSpread;
            ++i;
        }

        // today is a border case
        if (i < n && fixingDates[i] == today) {
            // might have been fixed
            try {
                Rate fixing = pastFixings[fixingDates[i]];
                if (fixing != Null<Real>()) {
                    const auto [gf, gfSpread] = growthFactor(fixing, i);
                    compoundFactorWithoutSpread *= gf;
                    compoundFactor *= gfSpread;
                    ++i;
                } else {
                    ; // fall through and forecast
                }
            } catch (Error&) {
                ; // fall through and forecast
            }
        }

        // forward part using telescopic property in order
        // to avoid the evaluation of multiple forward fixings
        // where possible.
        if (i < n) {
            const Handle<YieldTermStructure> curve = index->forwardingTermStructure();
            QL_REQUIRE(!curve.empty(),
                       "null term structure set to this instance of " << index->name());
            QL_REQUIRE(curve->referenceDate() <= valueDates[i] &&
                       (curve->allowsExtrapolation() || valueDates[n] <= curve->maxDate()),
                       "coupon requires a range [" << valueDates[i] << ", " << valueDates[n]
                       << "] wider than is supported by the term structure for instance of"
                       << index->name() << " [" << curve->referenceDate() << ", "
                       << (curve->allowsExtrapolation() ? Date::maxDate(): curve->maxDate()) << "]");

            const auto projGrowthFactor = [&](Size i) {
                return growthFactor(index->fixing(fixingDates[i]), i);
            };
            if (coupon_->canApplyTelescopicFormula()) {
                // handle partial accrual of first fixing if the first interestDate lands on a fixing holiday
                const Size telescopicStartIdx =
                    i == 0 && !applyObservationShift && (valueDates.front() < interestDates.front()) ? 1 : i;
                // telescopic formula up to potential lockout dates and partial accrual up to date.
                const Size endDateIdx = std::min<Size>(n, valueDates.size() - 1 - coupon_->lockoutDays());
                const Size telescopicEndIdx = endDateIdx - (applyObservationShift || valueDates[endDateIdx] <= date ? 0 : 1);
                if (telescopicStartIdx < telescopicEndIdx) {
                    while (i < telescopicStartIdx) {
                        // compound up any periods ahead of the telescopic range
                        const auto [gf, gfSpread] = projGrowthFactor(i);
                        compoundFactorWithoutSpread *= gf;
                        compoundFactor *= gfSpread;
                        ++i;
                    }
                    const DiscountFactor startDiscount = curve->discount(valueDates[telescopicStartIdx]);
                    const DiscountFactor endDiscount = curve->discount(valueDates[telescopicEndIdx]);
                    compoundFactor *= startDiscount / endDiscount;
                    compoundFactorWithoutSpread *= startDiscount / endDiscount;
                    i = telescopicEndIdx;
                }
            }
            // compound up any remaining periods
            while (i < n) {
                const auto [gf, gfSpread] = projGrowthFactor(i);
                compoundFactorWithoutSpread *= gf;
                compoundFactor *= gfSpread;
                ++i;
            }
        }
        const auto [rateAccrualStartDate, rateAccrualEndDate] = applyObservationShift
            ? std::make_pair(valueDates.front(), valueDates[n])
            : std::make_pair(interestDates.front(), std::min(date, interestDates[n]));
        const Rate tau = dc.yearFraction(rateAccrualStartDate, rateAccrualEndDate);
        const Rate rate = (compoundFactor - 1.0) / tau;
        Rate swapletRate = coupon_->gearing() * rate;
        Spread effectiveSpread;
        Rate effectiveIndexFixing;

        if (!compoundSpreadDaily) {
            swapletRate += couponSpread;
            effectiveSpread = couponSpread;
            effectiveIndexFixing = rate;
        } else {
            effectiveSpread = rate - (compoundFactorWithoutSpread - 1.0) / tau;
            effectiveIndexFixing = rate - effectiveSpread;
        }

        return std::make_tuple(swapletRate, effectiveSpread, effectiveIndexFixing);
    }

    Rate ArithmeticAveragedOvernightIndexedCouponPricer::swapletRate() const {
        return averageRate(coupon_->accrualEndDate());
    }

    Rate ArithmeticAveragedOvernightIndexedCouponPricer::averageRate(const Date& date) const {

        ext::shared_ptr<OvernightIndex> index =
            ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());

        const auto& fixingDates = coupon_->fixingDates();
        const auto& interestDates = coupon_->interestDates();
        const auto& dt = coupon_->dt();

        Size i = 0;
        const Size n = determineNumberOfFixings(interestDates, date);

        Real accumulatedRate = 0.0;

        const auto& pastFixings = index->timeSeries();

        // already fixed part
        Date today = Settings::instance().evaluationDate();
        while (i < n && fixingDates[i] < today) {
            // rate must have been fixed
            Rate pastFixing = pastFixings[fixingDates[i]];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << index->name() << " fixing for " << fixingDates[i]);
            Time span = (date >= interestDates[i + 1] ?
                         dt[i] :
                         index->dayCounter().yearFraction(interestDates[i], date));
            accumulatedRate += pastFixing * span;
            ++i;
        }

        // today is a border case
        if (i < n && fixingDates[i] == today) {
            // might have been fixed
            try {
                Rate pastFixing = pastFixings[fixingDates[i]];
                if (pastFixing != Null<Real>()) {
                    Time span = (date >= interestDates[i + 1] ?
                                 dt[i] :
                                 index->dayCounter().yearFraction(interestDates[i], date));
                    accumulatedRate += pastFixing * span;
                    ++i;
                } else {
                    ; // fall through and forecast
                }
            } catch (Error&) {
                ; // fall through and forecast
            }
        }

        /* forward part using telescopic property in order
        to avoid the evaluation of multiple forward fixings
        (approximation proposed by Katsumi Takada)*/
        if (byApprox_ && i < n) {
            Handle<YieldTermStructure> curve = index->forwardingTermStructure();
            QL_REQUIRE(!curve.empty(),
                       "null term structure set to this instance of " << index->name());

            const auto& dates = coupon_->valueDates();
            DiscountFactor startDiscount = curve->discount(dates[i]);
            DiscountFactor endDiscount = curve->discount(dates[n]);

            accumulatedRate +=
                log(startDiscount / endDiscount) -
                convAdj1(curve->timeFromReference(dates[i]), curve->timeFromReference(dates[n])) -
                convAdj2(curve->timeFromReference(dates[i]), curve->timeFromReference(dates[n]));
        }
        // otherwise
        else if (i < n) {
            Handle<YieldTermStructure> curve = index->forwardingTermStructure();
            QL_REQUIRE(!curve.empty(),
                       "null term structure set to this instance of " << index->name());

            const auto& dates = coupon_->valueDates();
            Time te = curve->timeFromReference(dates[n]);
            while (i < n) {
                // forcast fixing
                Rate forecastFixing = index->fixing(fixingDates[i]);
                Time ti1 = curve->timeFromReference(dates[i]);
                Time ti2 = curve->timeFromReference(dates[i + 1]);
                /*convexity adjustment due to payment dalay of each
                overnight fixing, supposing an Hull-White short rate model*/
                Real convAdj = exp(
                    0.5 * pow(vol_, 2.0) / pow(mrs_, 3.0) * (exp(2 * mrs_ * ti1) - 1) *
                    (exp(-mrs_ * ti2) - exp(-mrs_ * te)) * (exp(-mrs_ * ti2) - exp(-mrs_ * ti1)));
                accumulatedRate += convAdj * (1 + forecastFixing * dt[i]) - 1;
                ++i;
            }
        }

        Rate rate = accumulatedRate / coupon_->accruedPeriod(date);
        return coupon_->gearing() * rate + coupon_->spread();
    }

    Real ArithmeticAveragedOvernightIndexedCouponPricer::convAdj1(Time ts, Time te) const {
        return vol_ * vol_ / (4.0 * pow(mrs_, 3.0)) * (1.0 - exp(-2.0 * mrs_ * ts)) *
               pow((1.0 - exp(-mrs_ * (te - ts))), 2.0);
    }

    Real ArithmeticAveragedOvernightIndexedCouponPricer::convAdj2(Time ts, Time te) const {
        return vol_ * vol_ / (2.0 * pow(mrs_, 2.0)) *
               ((te - ts) - pow(1.0 - exp(-mrs_ * (te - ts)), 2.0) / mrs_ -
                (1.0 - exp(-2.0 * mrs_ * (te - ts))) / (2.0 * mrs_));
    }
}
