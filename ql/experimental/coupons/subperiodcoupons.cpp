/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Toyin Akin

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

#include <ql/experimental/coupons/subperiodcoupons.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <cmath>

namespace QuantLib {

    SubPeriodsCoupon::SubPeriodsCoupon(const Date& paymentDate,
                                       Real nominal,
                                       const Date& startDate,
                                       const Date& endDate,
                                       Natural fixingDays,
                                       const ext::shared_ptr<IborIndex>& index,
                                       Real gearing,
                                       Rate couponSpread,
                                       Rate rateSpread,
                                       const Date& refPeriodStart,
                                       const Date& refPeriodEnd,
                                       const DayCounter& dayCounter,
                                       const Date& exCouponDate)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, couponSpread,
                         refPeriodStart, refPeriodEnd, dayCounter, 
                         false, exCouponDate),
      rateSpread_(rateSpread) {
        Schedule sch = MakeSchedule()
                           .from(startDate)
                           .to(endDate)
                           .withTenor(index->tenor())
                           .withCalendar(index->fixingCalendar())
                           .withConvention(index->businessDayConvention())
                           .backwards()
                           .endOfMonth(index->endOfMonth());

        valueDates_ = sch.dates();

        // fixing dates
        n_ = valueDates_.size() - 1;
        if (index->fixingDays() == 0) {
            fixingDates_ = std::vector<Date>(valueDates_.begin(), valueDates_.end() - 1);
        } else {
            fixingDates_.resize(n_);
            for (Size i = 0; i < n_; ++i)
                fixingDates_[i] = index->fixingDate(valueDates_[i]);
        }

        // accrual periods
        dt_.resize(n_);
        const DayCounter& dc = index->dayCounter();
        for (Size i = 0; i < n_; ++i)
            dt_[i] = dc.yearFraction(valueDates_[i], valueDates_[i + 1]);
     }

    void SubPeriodsCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SubPeriodsCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    void SubPeriodsPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const SubPeriodsCoupon*>(&coupon);
        QL_REQUIRE(coupon_, "sub-periods coupon required");

        ext::shared_ptr<IborIndex> index = ext::dynamic_pointer_cast<IborIndex>(coupon_->index());
        if (!index) {
            // coupon was right, index is not
            QL_FAIL("IborIndex required");
        }

        accrualFactor_ = coupon_->accrualPeriod();
        QL_REQUIRE(accrualFactor_ != 0.0, "null accrual period");

        const std::vector<Date>& fixingDates = coupon_->fixingDates();
        Size n = fixingDates.size();
        subPeriodFixings_.resize(n);

        for (Size i = 0; i < n; i++) {
            subPeriodFixings_[i] = index->fixing(fixingDates[i]) + coupon_->rateSpread();
        }
    }

    Real SubPeriodsPricer::swapletPrice() const { 
        QL_FAIL("SubPeriodsPricer::swapletPrice not implemented");
    }

    Real SubPeriodsPricer::capletPrice(Rate) const {
        QL_FAIL("SubPeriodsPricer::capletPrice not implemented");
    }

    Rate SubPeriodsPricer::capletRate(Rate) const {
        QL_FAIL("SubPeriodsPricer::capletRate not implemented");
    }

    Real SubPeriodsPricer::floorletPrice(Rate) const {
        QL_FAIL("SubPeriodsPricer::floorletPrice not implemented");
    }

    Rate SubPeriodsPricer::floorletRate(Rate) const {
        QL_FAIL("SubPeriodsPricer::floorletRate not implemented");
    }

    Real AveragingRatePricer::swapletRate() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Size nCount = subPeriodFixings_.size();
        const std::vector<Time>& subPeriodFractions = coupon_->dt();
        Real aggregateFactor = 0.0;
        for (Size i = 0; i < nCount; i++) {
            aggregateFactor += subPeriodFixings_[i] * subPeriodFractions[i];
        }

        Real rate = aggregateFactor / coupon_->accrualPeriod();
        return coupon_->gearing() * rate + coupon_->spread();
    }

    Real CompoundingRatePricer::swapletRate() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Real compoundFactor = 1.0;
        const std::vector<Time>& subPeriodFractions = coupon_->dt();
        Size nCount = subPeriodFixings_.size();
        for (Size i = 0; i < nCount; i++) {
            compoundFactor *= (1.0 + subPeriodFixings_[i] * subPeriodFractions[i]);
        }

        Real rate = (compoundFactor - 1.0) / coupon_->accrualPeriod();
        return coupon_->gearing() * rate + coupon_->spread();
    }

}

