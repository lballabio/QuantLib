/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file capflooredinflationcoupon.hpp
 \brief caplet and floorlet pricing for YoY inflation coupons
 */

#ifndef quantlib_capfloored_inflation_coupon_hpp
#define quantlib_capfloored_inflation_coupon_hpp

#include <ql/cashflows/yoyinflationcoupon.hpp>

namespace QuantLib {

    //! Capped or floored inflation coupon.
    /*! Essentially a copy of the nominal version but taking a
        different index and a set of pricers (not just one).

        The payoff \f$ P \f$ of a capped inflation-rate coupon
        with paysWithin = true is:

        \f[ P = N \times T \times \min(a L + b, C). \f]

        where \f$ N \f$ is the notional, \f$ T \f$ is the accrual
        time, \f$ L \f$ is the inflation rate, \f$ a \f$ is its
        gearing, \f$ b \f$ is the spread, and \f$ C \f$ and \f$ F \f$
        the strikes.

        The payoff of a floored inflation-rate coupon is:

        \f[ P = N \times T \times \max(a L + b, F). \f]

        The payoff of a collared inflation-rate coupon is:

        \f[ P = N \times T \times \min(\max(a L + b, F), C). \f]

        If paysWithin = false then the inverse is returned
        (this provides for instrument cap and caplet prices).

        They can be decomposed in the following manner.  Decomposition
        of a capped floating rate coupon when paysWithin = true:
        \f[
        R = \min(a L + b, C) = (a L + b) + \min(C - b - \xi |a| L, 0)
        \f]
        where \f$ \xi = sgn(a) \f$. Then:
        \f[
        R = (a L + b) + |a| \min(\frac{C - b}{|a|} - \xi L, 0)
        \f]
     */
    class CappedFlooredYoYInflationCoupon : public YoYInflationCoupon {
      public:
        // we may watch an underlying coupon ...
        CappedFlooredYoYInflationCoupon(
                const ext::shared_ptr<YoYInflationCoupon>& underlying,
                Rate cap = Null<Rate>(),
                Rate floor = Null<Rate>());

        // ... or not
        CappedFlooredYoYInflationCoupon(const Date& paymentDate,
                                        Real nominal,
                                        const Date& startDate,
                                        const Date& endDate,
                                        Natural fixingDays,
                                        const ext::shared_ptr<YoYInflationIndex>& index,
                                        const Period& observationLag,
                                        const DayCounter& dayCounter,
                                        Real gearing = 1.0,
                                        Spread spread = 0.0,
                                        const Rate cap = Null<Rate>(),
                                        const Rate floor = Null<Rate>(),
                                        const Date& refPeriodStart = Date(),
                                        const Date& refPeriodEnd = Date())
        : YoYInflationCoupon(paymentDate, nominal, startDate, endDate,
                             fixingDays, index, observationLag,  dayCounter,
                             gearing, spread, refPeriodStart, refPeriodEnd),
          isFloored_(false), isCapped_(false) {
            setCommon(cap, floor);
        }

        //! \name augmented Coupon interface
        //@{
        //! swap(let) rate
        Rate rate() const override;
        //! cap
        Rate cap() const;
        //! floor
        Rate floor() const;
        //! effective cap of fixing
        Rate effectiveCap() const;
        //! effective floor of fixing
        Rate effectiveFloor() const;
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor& v) override;
        //@}

        //! this returns the expected rate before cap and floor are applied
        Rate underlyingRate() const;

        bool isCapped() const { return isCapped_; }
        bool isFloored() const { return isFloored_; }

        void setPricer(const ext::shared_ptr<YoYInflationCouponPricer>&);

      protected:
        // data, we only use underlying_ if it was constructed that way,
        // generally we use the shared_ptr conversion to boolean to test
        ext::shared_ptr<YoYInflationCoupon> underlying_;
        bool isFloored_, isCapped_;
        Rate cap_, floor_;
      private:
        virtual void setCommon(Rate cap, Rate floor);
    };

}

#endif

