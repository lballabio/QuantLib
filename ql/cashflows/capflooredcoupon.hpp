/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2006 Cristina Duminuco

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

/*! \file capflooredcoupon.hpp
    \brief Floating rate coupon with additional cap/floor
*/

#ifndef quantlib_capped_floored_coupon_hpp
#define quantlib_capped_floored_coupon_hpp

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {
    class Date;
    //! Capped and/or floored floating-rate coupon
    /*! The payoff \f$ P \f$ of a capped floating-rate coupon is:
        \f[ P = N \times T \times \min(a L + b, C). \f]
        The payoff of a floored floating-rate coupon is:
        \f[ P = N \times T \times \max(a L + b, F). \f]
        The payoff of a collared floating-rate coupon is:
        \f[ P = N \times T \times \min(\max(a L + b, F), C). \f]

        where \f$ N \f$ is the notional, \f$ T \f$ is the accrual
        time, \f$ L \f$ is the floating rate, \f$ a \f$ is its
        gearing, \f$ b \f$ is the spread, and \f$ C \f$ and \f$ F \f$
        the strikes.

        They can be decomposed in the following manner.
        Decomposition of a capped floating rate coupon:
        \f[
        R = \min(a L + b, C) = (a L + b) + \min(C - b - \xi |a| L, 0)
        \f]
        where \f$ \xi = sgn(a) \f$. Then:
        \f[
        R = (a L + b) + |a| \min(\frac{C - b}{|a|} - \xi L, 0)
        \f]
    */
    class CappedFlooredCoupon : public FloatingRateCoupon {
      public:
        CappedFlooredCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>());
        //! \name Coupon interface
        //@{
        Rate rate() const;
        Rate convexityAdjustment() const;
        //@}
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
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);

        bool isCapped() const {return isCapped_;}
        bool isFloored() const {return isFloored_;}

        void setPricer(
                   const boost::shared_ptr<FloatingRateCouponPricer>& pricer);

    protected:
        // data
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        bool isCapped_, isFloored_;
        Rate cap_, floor_;
    };

    class CappedFlooredIborCoupon : public CappedFlooredCoupon {
      public:
        CappedFlooredIborCoupon(
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const boost::shared_ptr<IborIndex>& index,
                  Real gearing = 1.0,
                  Spread spread = 0.0,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false)
        : CappedFlooredCoupon(boost::shared_ptr<FloatingRateCoupon>(new
            IborCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                       index, gearing, spread, refPeriodStart, refPeriodEnd,
                       dayCounter, isInArrears)), cap, floor) {}

        virtual void accept(AcyclicVisitor& v) {
            Visitor<CappedFlooredIborCoupon>* v1 =
                dynamic_cast<Visitor<CappedFlooredIborCoupon>*>(&v);
            if (v1 != 0)
                v1->visit(*this);
            else
                CappedFlooredCoupon::accept(v);
        }
    };

    class CappedFlooredCmsCoupon : public CappedFlooredCoupon {
      public:
        CappedFlooredCmsCoupon(
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const boost::shared_ptr<SwapIndex>& index,
                  Real gearing = 1.0,
                  Spread spread= 0.0,
                  const Rate cap = Null<Rate>(),
                  const Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false)
        : CappedFlooredCoupon(boost::shared_ptr<FloatingRateCoupon>(new
            CmsCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                      index, gearing, spread, refPeriodStart, refPeriodEnd,
                      dayCounter, isInArrears)), cap, floor) {}

        virtual void accept(AcyclicVisitor& v) {
            Visitor<CappedFlooredCmsCoupon>* v1 =
                dynamic_cast<Visitor<CappedFlooredCmsCoupon>*>(&v);
            if (v1 != 0)
                v1->visit(*this);
            else
                CappedFlooredCoupon::accept(v);
        }
    };

}

#endif
