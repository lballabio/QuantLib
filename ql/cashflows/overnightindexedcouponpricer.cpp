/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
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

using std::vector;

namespace QuantLib {

    namespace {

        Size determineNumberOfFixings(const vector<Date>& interestDates,
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
        const auto& pastFixings = IndexManager::instance().getHistory(index->name());

        const vector<Date>& fixingDates = coupon_->fixingDates();
        const vector<Date>& valueDates = coupon_->valueDates();
        const vector<Date>& interestDates = coupon_->interestDates();
        const vector<Time>& dt = coupon_->dt();
        const bool applyObservationShift = coupon_->applyObservationShift();

        Size i = 0;
        const Size n = determineNumberOfFixings(interestDates, date, applyObservationShift);

        Real compoundFactor = 1.0;

        // already fixed part
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
}
