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
                                       const Date& refPeriodStart,
                                       const Date& refPeriodEnd,
                                       const DayCounter& dayCounter,
                                       bool isInArrears,
                                       const Date& exCouponDate,
                                       Rate rateSpread)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, couponSpread,
                         refPeriodStart, refPeriodEnd, dayCounter, 
                         isInArrears, exCouponDate),
      rateSpread_(rateSpread) {
        Schedule sch = MakeSchedule()
                           .from(startDate)
                           .to(endDate)
                           .withTenor(index->tenor())
                           .withCalendar(index->fixingCalendar())
                           .withConvention(index->businessDayConvention())
                           .backwards()
                           .endOfMonth(index->endOfMonth());

        observationDates_ = sch.dates();
        observations_ = observationDates_.size();
     }

    void SubPeriodsCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SubPeriodsCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    void SubPeriodsPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ =  dynamic_cast<const SubPeriodsCoupon*>(&coupon);
        QL_REQUIRE(coupon_, "sub-periods coupon required");
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();

        Date paymentDate = coupon_->date();

        ext::shared_ptr<IborIndex> index =
            ext::dynamic_pointer_cast<IborIndex>(coupon_->index());
        const Handle<YieldTermStructure>& rateCurve =
            index->forwardingTermStructure();
        discount_ = rateCurve->discount(paymentDate);
        
        accrualFactor_ = coupon_->accrualPeriod();
        spreadLegValue_ = spread_ * accrualFactor_ * discount_;
        Size observations = coupon_->observations();

        const std::vector<Date>& observationDates = coupon_->observationDates();

        initialValues_ = std::vector<Real>(observations - 1, 0.);
        observationCvg_ = std::vector<Real>(observations - 1, 0.);

        for (Size i = 0; i < observations - 1; i++) {
            Date refDate = coupon_->isInArrears() ? observationDates[i + 1] : observationDates[i];
            Date fixingDate = index->fixingDate(refDate);

            initialValues_[i] =
                index->fixing(fixingDate) + coupon_->rateSpread();

            observationCvg_[i] =
                index->dayCounter().yearFraction(observationDates[i], observationDates[i + 1]);
        }
    }

    Real SubPeriodsPricer::swapletRate() const {
        return swapletPrice() / (accrualFactor_ * discount_);
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

    Real AveragingRatePricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Size nCount = initialValues_.size();
        Real dAvgRate = 0.0, dTotalCvg = 0.0, dTotalPayment = 0.0;
        for (Size i=0; i<nCount; i++) {
            dTotalPayment += initialValues_[i] * observationCvg_[i];
            dTotalCvg += observationCvg_[i];
        }

        dAvgRate = dTotalPayment / dTotalCvg;

        Real swapletPrice = dAvgRate*coupon_->accrualPeriod()*discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

    Real CompoundingRatePricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Real dNotional = 1.0;

        Size nCount = initialValues_.size();
        Real dCompRate = 0.0, dTotalCvg = 0.0, dTotalPayment = 0.0;
        for (Size i=0; i<nCount; i++) {
            dTotalPayment = initialValues_[i] * observationCvg_[i] * dNotional;
            dNotional += dTotalPayment;
            dTotalCvg += observationCvg_[i];
        }

        dCompRate = (dNotional - 1.0)/dTotalCvg;

        Real swapletPrice = dCompRate*coupon_->accrualPeriod()*discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

}

