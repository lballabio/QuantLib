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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/experimental/averageois/averageoiscouponpricer.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/vectors.hpp>
#include <utility>
#include <algorithm>

using std::vector;

namespace QuantLib {

    namespace {

        Size determineNumberOfFixings(const vector<Date>& interestDates,
                                      const Date& date,
                                      bool applyObservationShift) {
            Size n =
                std::lower_bound(interestDates.begin(), interestDates.end(), date) - interestDates.begin();
            // When using the observation shift, it may happen that
            // that the end of accrual period will fall later than the last
            // interest date. In which case, n will be equal to the number of
            // interest dates, while we know that the number of fixing dates is
            // always one less than the number of interest dates.
            return n == interestDates.size() && applyObservationShift ? n - 1 : n;
        }

        Date applyLookbackPeriod(const ext::shared_ptr<InterestRateIndex>& index,
                                 const Date& valueDate,
                                 Natural lookbackDays) {
            return index->fixingCalendar().advance(valueDate, -static_cast<Integer>(lookbackDays),
                                                   Days);
        }
    }

    OvernightIndexedCoupon::OvernightIndexedCoupon(
                    const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const ext::shared_ptr<OvernightIndex>& overnightIndex,
                    Real gearing,
                    Spread spread,
                    const Date& refPeriodStart,
                    const Date& refPeriodEnd,
                    const DayCounter& dayCounter,
                    bool telescopicValueDates,
                    RateAveraging::Type averagingMethod,
                    Natural lookbackDays,
                    Natural lockoutDays,
                    bool applyObservationShift)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         lookbackDays,
                         overnightIndex,
                         gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, false), 
        averagingMethod_(averagingMethod), lockoutDays_(lockoutDays),
        applyObservationShift_(applyObservationShift) {

        // value dates
        Date tmpEndDate = endDate;

        /* For the coupon's valuation only the first and last future valuation
           dates matter, therefore we can avoid to construct the whole series
           of valuation dates, a front and back stub will do. However notice
           that if the global evaluation date moves forward it might run past
           the front stub of valuation dates we build here (which incorporates
           a grace period of 7 business after the evaluation date). This will
           lead to false coupon projections (see the warning the class header). */

        QL_REQUIRE(canApplyTelescopicFormula() || !telescopicValueDates,
                   "Telescopic formula cannot be applied for a coupon with lookback.");

        if (telescopicValueDates) {
            // build optimised value dates schedule: front stub goes
            // from start date to max(evalDate,startDate) + 7bd
            Date evalDate = Settings::instance().evaluationDate();
            tmpEndDate = overnightIndex->fixingCalendar().advance(
                std::max(startDate, evalDate), 7, Days, Following);
            tmpEndDate = std::min(tmpEndDate, endDate);
        }
        Schedule sch =
            MakeSchedule()
                .from(startDate)
                // .to(endDate)
                .to(tmpEndDate)
                .withTenor(1 * Days)
                .withCalendar(overnightIndex->fixingCalendar())
                .withConvention(overnightIndex->businessDayConvention())
                .backwards();
        valueDates_ = sch.dates();

        if (telescopicValueDates) {
            // if lockout days are defined, we need to ensure that
            // the lockout period is covered by the value dates
            tmpEndDate = overnightIndex->fixingCalendar().adjust(
                endDate, overnightIndex->businessDayConvention());
            Date tmpLockoutDate = overnightIndex->fixingCalendar().advance(
                endDate, -std::max<Integer>(lockoutDays_, 1), Days, Preceding);
            while (tmpLockoutDate <= tmpEndDate)
            {
                if (tmpLockoutDate > valueDates_.back())
                    valueDates_.push_back(tmpLockoutDate);
                tmpLockoutDate =
                    overnightIndex->fixingCalendar().advance(tmpLockoutDate, 1, Days, Following);
            }
        }

        QL_ENSURE(valueDates_.size()>=2, "degenerate schedule");

        n_ = valueDates_.size() - 1;

        interestDates_ = vector<Date>(valueDates_.begin(), valueDates_.end());

        if (fixingDays_ == overnightIndex->fixingDays() && fixingDays_ == 0) {
            fixingDates_ = vector<Date>(valueDates_.begin(), valueDates_.end() - 1);
        } else {
            // Lookback (fixing days) without observation shift:
            // The date that the fixing rate is pulled  from (the observation date) is k
            // business days before the date that interest is applied (the interest date)
            // and is applied for the number of calendar days until the next business
            // day following the interest date.
            fixingDates_.resize(n_);
            for (Size i = 0; i <= n_; ++i) {
                Date tmp = applyLookbackPeriod(overnightIndex, valueDates_[i], fixingDays_);
                if (i < n_)
                    fixingDates_[i] = tmp;
                if (applyObservationShift_)
                    // Lookback (fixing days) with observation shift:
                    // The date that the fixing rate is pulled from (the observation date)
                    // is k business days before the date that interest is applied
                    // (the interest date) and is applied for the number of calendar
                    // days until the next business day following the observation date.
                    // This means that the fixing dates periods align with value dates.
                    interestDates_[i] = tmp;
                if (fixingDays_ != overnightIndex->fixingDays())
                    // If fixing dates of the coupon deviate from fixing days in the index
                    // we need to correct the value dates such that they reflect dates
                    // corresponding to a deposit instrument linked to the index.
                    // This is to ensure that future projections (which are computed
                    // based on the value dates) of the index do not
                    // yield any convexity corrections.
                    valueDates_[i] = overnightIndex->valueDate(tmp);
            }
        }
        // When lockout is used the fixing rate applied for the last k days of the
        // interest period is frozen at the rate observed k days before the period ends.
        if (lockoutDays_ != 0) {
            QL_REQUIRE(lockoutDays_ > 0 && lockoutDays_ < n_,
                       "Lockout period cannot be negative or exceed the number of fixing days.");
            Date lockoutDate = fixingDates_[n_ - 1 - lockoutDays_];
            for (Size i = n_ - 1; i > n_ - 1 - lockoutDays_; --i)
                fixingDates_[i] = lockoutDate;
        }

        // accrual (compounding) periods
        dt_.resize(n_);
        const DayCounter& dc = overnightIndex->dayCounter();
        for (Size i=0; i<n_; ++i)
            dt_[i] = dc.yearFraction(interestDates_[i], interestDates_[i + 1]);

        switch (averagingMethod) {
            case RateAveraging::Simple:
                QL_REQUIRE(
                    fixingDays_ == overnightIndex->fixingDays() && !applyObservationShift_ &&
                        lockoutDays_ == 0,
                    "Cannot price an overnight coupon with simple averaging with lookback or "
                    "lockout.");
                setPricer(ext::shared_ptr<FloatingRateCouponPricer>(
                    new ArithmeticAveragedOvernightIndexedCouponPricer(telescopicValueDates)));
                break;
            case RateAveraging::Compound:
                setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new OvernightIndexedCouponPricer));
                break;
            default:
                QL_FAIL("unknown compounding convention (" << Integer(averagingMethod) << ")");
        }
    }

    Real OvernightIndexedCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            // out of coupon range
            return 0.0;
        } else if (tradingExCoupon(d)) {
            return nominal() * averageRate(d) * accruedPeriod(d);
        } else {
            // usual case
            return nominal() * averageRate(std::min(d, accrualEndDate_)) * accruedPeriod(d);
        }
    }

    Rate OvernightIndexedCoupon::averageRate(const Date& d) const {
        QL_REQUIRE(pricer_, "pricer not set");
        pricer_->initialize(*this);
        const auto overnightIndexPricer = ext::dynamic_pointer_cast<OvernightIndexedCouponPricer>(pricer_);
        if (overnightIndexPricer)
            return overnightIndexPricer->averageRate(d);

        return pricer_->swapletRate();
    }

    const vector<Rate>& OvernightIndexedCoupon::indexFixings() const {
        fixings_.resize(n_);
        for (Size i=0; i<n_; ++i)
            fixings_[i] = index_->fixing(fixingDates_[i]);
        return fixings_;
    }

    void OvernightIndexedCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightIndexedCoupon>*>(&v);
        if (v1 != nullptr) {
            v1->visit(*this);
        } else {
            FloatingRateCoupon::accept(v);
        }
    }

    void OvernightIndexedCouponPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
        QL_ENSURE(coupon_, "wrong coupon type");
    }

    Rate OvernightIndexedCouponPricer::swapletRate() const {
        return averageRate(coupon_->accrualEndDate());
    }

    Rate OvernightIndexedCouponPricer::averageRate(const Date& date) const {
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

    OvernightLeg::OvernightLeg(Schedule schedule, ext::shared_ptr<OvernightIndex> i)
    : schedule_(std::move(schedule)), overnightIndex_(std::move(i)), paymentCalendar_(schedule_.calendar()) {
        QL_REQUIRE(overnightIndex_, "no index provided");
    }

    OvernightLeg& OvernightLeg::withNotionals(Real notional) {
        notionals_ = vector<Real>(1, notional);
        return *this;
    }

    OvernightLeg& OvernightLeg::withNotionals(const vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    OvernightLeg& OvernightLeg::withPaymentDayCounter(const DayCounter& dc) {
        paymentDayCounter_ = dc;
        return *this;
    }

    OvernightLeg&
    OvernightLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    OvernightLeg& OvernightLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    OvernightLeg& OvernightLeg::withPaymentLag(Integer lag) {
        paymentLag_ = lag;
        return *this;
    }

    OvernightLeg& OvernightLeg::withGearings(Real gearing) {
        gearings_ = vector<Real>(1,gearing);
        return *this;
    }

    OvernightLeg& OvernightLeg::withGearings(const vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    OvernightLeg& OvernightLeg::withSpreads(Spread spread) {
        spreads_ = vector<Spread>(1,spread);
        return *this;
    }

    OvernightLeg& OvernightLeg::withSpreads(const vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    OvernightLeg& OvernightLeg::withTelescopicValueDates(bool telescopicValueDates) {
        telescopicValueDates_ = telescopicValueDates;
        return *this;
    }

    OvernightLeg& OvernightLeg::withAveragingMethod(RateAveraging::Type averagingMethod) {
        averagingMethod_ = averagingMethod;
        return *this;
    }

    OvernightLeg& OvernightLeg::withLookbackDays(Natural lookbackDays) {
        lookbackDays_ = lookbackDays;
        return *this;
    }
    OvernightLeg& OvernightLeg::withLockoutDays(Natural lockoutDays) {
        lockoutDays_ = lockoutDays;
        return *this;
    }
    OvernightLeg& OvernightLeg::withObservationShift(bool applyObservationShift) {
        applyObservationShift_ = applyObservationShift;
        return *this;
    }

    OvernightLeg::operator Leg() const {

        QL_REQUIRE(!notionals_.empty(), "no notional given");

        Leg cashflows;

        // the following is not always correct
        Calendar calendar = schedule_.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;

        Size n = schedule_.size()-1;
        for (Size i=0; i<n; ++i) {
            refStart = start = schedule_.date(i);
            refEnd   =   end = schedule_.date(i+1);
            paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);

            if (i == 0 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1))
                refStart = calendar.adjust(end - schedule_.tenor(),
                                           paymentAdjustment_);
            if (i == n-1 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule_.tenor(),
                                         paymentAdjustment_);

            cashflows.push_back(ext::shared_ptr<CashFlow>(new
                OvernightIndexedCoupon(paymentDate,
                                       detail::get(notionals_, i,
                                                   notionals_.back()),
                                       start, end,
                                       overnightIndex_,
                                       detail::get(gearings_, i, 1.0),
                                       detail::get(spreads_, i, 0.0),
                                       refStart, refEnd,
                                       paymentDayCounter_,
                                       telescopicValueDates_,
                                       averagingMethod_,
                                       lookbackDays_,
                                       lockoutDays_,
                                       applyObservationShift_)));
        }
        return cashflows;
    }

}
