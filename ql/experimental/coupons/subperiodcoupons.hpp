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

/*! \file subperiodcoupons.hpp
    \brief averaging coupons
*/

#ifndef quantlib_sub_period_coupons_hpp
#define quantlib_sub_period_coupons_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/time/schedule.hpp>
#include <vector>

namespace QuantLib {

    class IborIndex;

    class SubPeriodsCoupon: public FloatingRateCoupon {
      public:
          // The index object passed in has a tenor significantly less than the
          // start/end dates.
          // Thus endDate-startDate may equal 3M
          // The Tenor used within the index object should be 1M for
          // averaging/compounding across three coupons within the
          // coupon period.
        SubPeriodsCoupon(const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         Natural fixingDays,
                         const ext::shared_ptr<IborIndex>& index,
                         Real gearing = 1.0,
                         Rate couponSpread = 0.0, // Spread added to the computed
                                                  // averaging/compounding rate.
                         Rate rateSpread = 0.0,   // Spread to be added onto each
                                                  // fixing within the
                                                  // averaging/compounding calculation
                         const Date& refPeriodStart = Date(),
                         const Date& refPeriodEnd = Date(),
                         const DayCounter& dayCounter = DayCounter(),
                         const Date& exCouponDate = Date());

        Spread rateSpread() const { return rateSpread_; }

        Size observations() const { return observations_; }
        
        const std::vector<Date>& observationDates() const {
            return observationDates_;
        }

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        std::vector<Date> observationDates_;
        Size observations_;
        Rate rateSpread_;
    };

    class SubPeriodsPricer: public FloatingRateCouponPricer {
      public:
        Rate swapletPrice() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        void initialize(const FloatingRateCoupon& coupon) override;

      protected:
        const SubPeriodsCoupon* coupon_;
        Real accrualFactor_;
        std::vector<Real> subPeriodFractions_;
        std::vector<Real> subPeriodFixings_;
    };

    class AveragingRatePricer: public SubPeriodsPricer {
      public:
        Real swapletRate() const override;
    };

    class CompoundingRatePricer: public SubPeriodsPricer {
      public:
        Real swapletRate() const override;
    };
}

#endif
