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
#include <ql/cashflows/overnightindexedcouponpricer.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/ibor/cdi.hpp>
#include <ql/utilities/vectors.hpp>
#include <utility>
#include <algorithm>

using std::vector;

namespace QuantLib {

    namespace {
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
      applyObservationShift_(applyObservationShift), isCdiIndexed_(false) {

        // value dates
        Date tmpEndDate = endDate;

        /* For the coupon's valuation only the first and last future valuation
           dates matter, therefore we can avoid to construct the whole series
           of valuation dates, a front and back stub will do. However, notice
           that if the global evaluation date moves forward it might run past
           the front stub of valuation dates we build here (which incorporates
           a grace period of 7 business after the evaluation date). This will
           lead to false coupon projections (see the warning the class header). */

        QL_REQUIRE(!telescopicValueDates || canApplyTelescopicFormula(),
                   "Telescopic formula cannot be applied for a coupon with lookback "
                   "and/or a cdi coupon with gearing.");

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

        if (ext::dynamic_pointer_cast<Cdi>(index_) != nullptr) {
            QL_REQUIRE(averagingMethod == RateAveraging::Compound,
                       "Cdi-indexed coupon not implemented for "
                       "RateAveraging method other than Compound.");
            isCdiIndexed_ = true;
        }

        switch (averagingMethod) {
            case RateAveraging::Simple:
            QL_REQUIRE(
                fixingDays_ == overnightIndex->fixingDays() && !applyObservationShift_ && lockoutDays_ == 0,
                "Cannot price an overnight coupon with simple averaging with lookback or lockout.");
            setPricer(ext::make_shared<ArithmeticAveragedOvernightIndexedCouponPricer>(telescopicValueDates));
                break;
            case RateAveraging::Compound:
                setPricer(ext::make_shared<CompoundingOvernightIndexedCouponPricer>());
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
        if (const auto compoundingPricer =
                ext::dynamic_pointer_cast<CompoundingOvernightIndexedCouponPricer>(pricer_)) {
            return compoundingPricer->averageRate(d);
        }
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

            const auto overnightIndexedCoupon = ext::make_shared<OvernightIndexedCoupon>(
                paymentDate, detail::get(notionals_, i, notionals_.back()), start, end,
                overnightIndex_, detail::get(gearings_, i, 1.0), detail::get(spreads_, i, 0.0),
                refStart, refEnd, paymentDayCounter_, telescopicValueDates_, averagingMethod_,
                lookbackDays_, lockoutDays_, applyObservationShift_);

            cashflows.push_back(overnightIndexedCoupon);
        }
        return cashflows;
    }

}
