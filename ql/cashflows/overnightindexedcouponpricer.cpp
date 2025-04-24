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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/overnightindexedcouponpricer.hpp>

namespace QuantLib {

    namespace {

        Size determineNumberOfFixings(const std::vector<Date>& interestDates,
                                      const Date& date,
                                      bool applyObservationShift) {
            Size n = std::lower_bound(interestDates.begin(), interestDates.end(), date) -
                     interestDates.begin();
            // When using the observation shift, it may happen that
            // that the end of accrual period will fall later than the last
            // interest date. In which case, n will be equal to the number of
            // interest dates, while we know that the number of fixing dates is
            // always one less than the number of interest dates.
            return n == interestDates.size() && applyObservationShift ? n - 1 : n;
        }
    }

    void CompoundingOvernightIndexedCouponPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
        QL_ENSURE(coupon_, "wrong coupon type");
    }

    Rate CompoundingOvernightIndexedCouponPricer::swapletRate() const {
        return averageRate(coupon_->accrualEndDate());
    }

    Rate CompoundingOvernightIndexedCouponPricer::averageRate(const Date& date) const {
        const Date today = Settings::instance().evaluationDate();

        const ext::shared_ptr<OvernightIndex> index =
            ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());
        const auto& pastFixings = index->timeSeries();
        const auto& fixingCalendar = index->fixingCalendar();

        const auto& fixingDates = coupon_->fixingDates();
        const auto& valueDates = coupon_->valueDates();
        const auto& interestDates = coupon_->interestDates();
        const auto& dt = coupon_->dt();
        const bool applyObservationShift = coupon_->applyObservationShift();

        Size i = 0;
        const Size n = determineNumberOfFixings(interestDates, date, applyObservationShift);

        Real compoundFactor = 1.0;
        Date lastPastFixingDate = n < fixingCalendar.businessDaysBetween(fixingDates[0], today)
            ? fixingCalendar.advance(fixingDates[0], Period(n, Days)) : fixingCalendar.advance(today, Period(-1, Days));
        Real indexCompoundedFactor = index->compoundedFactor(fixingDates[0], lastPastFixingDate);
        auto calculatePastFixing = [&](){
            while (i < n && fixingDates[i] < today) {
                // rate must have been fixed
                const Rate fixing = pastFixings[fixingDates[i]];
                QL_REQUIRE(fixing != Null<Real>(),
                           "Missing " << index->name() << " fixing for " << fixingDates[i]);
                Time span = (date >= interestDates[i + 1] ?
                                 dt[i] :
                                 index->dayCounter().yearFraction(interestDates[i], date));
                compoundFactor *= (1.0 + fixing * span);
                ++i;
            }    
        };

        if (indexCompoundedFactor == Null<Real>() 
            || applyObservationShift 
            || coupon_->lockoutDays() != Null<Natural>()) {
            calculatePastFixing();
        } else {
            compoundFactor = indexCompoundedFactor;
            i = fixingCalendar.businessDaysBetween(fixingDates[0], 
                                                   lastPastFixingDate,
                                                   true, true);
        }

        // today is a border case
        if (i < n && fixingDates[i] == today) {
            // might have been fixed
            try {
                Rate fixing = pastFixings[fixingDates[i]];
                if (fixing != Null<Real>()) {
                    Time span = (date >= interestDates[i + 1] ?
                                     dt[i] :
                                     index->dayCounter().yearFraction(interestDates[i], date));
                    compoundFactor *= (1.0 + fixing * span);
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

            const auto effectiveRate = [&index, &fixingDates, &date, &interestDates,
                                        &dt](Size position) {
                Rate fixing = index->fixing(fixingDates[position]);
                Time span = (date >= interestDates[position + 1] ?
                                 dt[position] :
                                 index->dayCounter().yearFraction(interestDates[position], date));
                return span * fixing;
            };

            if (!coupon_->canApplyTelescopicFormula()) {
                // With lookback applied, the telescopic formula cannot be used,
                // we need to project each fixing in the coupon.
                // Only in one particular case when observation shift is used and
                // no intrinsic index fixing delay is applied, the telescopic formula
                // holds, because regardless of the fixing delay in the coupon,
                // in such configuration value dates will be equal to interest dates.
                // A potential lockout, which may occur in tandem with a lookback
                // setting, will be handled automatically based on fixing dates.
                // Same applies to a case when accrual calculation date does or
                // does not occur on an interest date.
                while (i < n) {
                    compoundFactor *= (1.0 + effectiveRate(i));
                    ++i;
                }
            } else {
                // No lookback, we can partially apply the telescopic formula.
                // But we need to make a correction for a potential lockout.
                const Size nLockout = n - coupon_->lockoutDays();
                const bool isLockoutApplied = coupon_->lockoutDays() > 0;

                // Lockout could already start at or before i.
                // In such case the ratio of discount factors will be equal to 1.
                const DiscountFactor startDiscount =
                    curve->discount(valueDates[std::min<Size>(nLockout, i)]);
                if (interestDates[n] == date || isLockoutApplied) {
                    // telescopic formula up to potential lockout dates.
                    const DiscountFactor endDiscount =
                        curve->discount(valueDates[std::min<Size>(nLockout, n)]);
                    compoundFactor *= startDiscount / endDiscount;
                    // For the lockout periods the telescopic formula does not apply.
                    // The value dates (at which the projection is calculated) correspond
                    // to the locked-out fixing, while the interest dates (at which the
                    // interest over that fixing is accrued) are not fixed at lockout,
                    // hence they do not cancel out.
                    i = std::max(nLockout, i);

                    // With no lockout, the loop is skipped because i = n.
                    while (i < n) {
                        compoundFactor *= (1.0 + effectiveRate(i));
                        ++i;
                    }
                } else {
                    // No lockout and date is different than last interest date.
                    // The last fixing is not used for its full period (the date is between
                    // its start and end date).  We can use the telescopic formula until the
                    // previous date, then we'll add the missing bit.
                    const DiscountFactor endDiscount = curve->discount(valueDates[n - 1]);
                    compoundFactor *= startDiscount / endDiscount;
                    compoundFactor *= (1.0 + effectiveRate(n - 1));
                }
            }
        }

        const Rate rate = (compoundFactor - 1.0) / coupon_->accruedPeriod(date);
        return coupon_->gearing() * rate + coupon_->spread();
    }

    void
    ArithmeticAveragedOvernightIndexedCouponPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
        QL_ENSURE(coupon_, "wrong coupon type");
    }

    Rate ArithmeticAveragedOvernightIndexedCouponPricer::swapletRate() const {

        ext::shared_ptr<OvernightIndex> index =
            ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());

        const auto& fixingDates = coupon_->fixingDates();
        const auto& dt = coupon_->dt();

        Size n = dt.size(), i = 0;

        Real accumulatedRate = 0.0;

        const auto& pastFixings = index->timeSeries();

        // already fixed part
        Date today = Settings::instance().evaluationDate();
        while (i < n && fixingDates[i] < today) {
            // rate must have been fixed
            Rate pastFixing = pastFixings[fixingDates[i]];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << index->name() << " fixing for " << fixingDates[i]);
            accumulatedRate += pastFixing * dt[i];
            ++i;
        }

        // today is a border case
        if (i < n && fixingDates[i] == today) {
            // might have been fixed
            try {
                Rate pastFixing = pastFixings[fixingDates[i]];
                if (pastFixing != Null<Real>()) {
                    accumulatedRate += pastFixing * dt[i];
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

        Rate rate = accumulatedRate / coupon_->accrualPeriod();
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
