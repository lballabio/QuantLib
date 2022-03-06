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
                  const ext::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>());
        //! \name Coupon interface
        //@{
        Rate rate() const override;
        Rate convexityAdjustment() const override;
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
        void update() override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;

        bool isCapped() const {return isCapped_;}
        bool isFloored() const {return isFloored_;}

        void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& pricer) override;

        ext::shared_ptr<FloatingRateCoupon> underlying() { return underlying_; }

      protected:
        // data
        ext::shared_ptr<FloatingRateCoupon> underlying_;
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
                  const ext::shared_ptr<IborIndex>& index,
                  Real gearing = 1.0,
                  Spread spread = 0.0,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false,
                  const Date& exCouponDate = Date())
        : CappedFlooredCoupon(ext::shared_ptr<FloatingRateCoupon>(new
            IborCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                       index, gearing, spread, refPeriodStart, refPeriodEnd,
                       dayCounter, isInArrears, exCouponDate)), cap, floor) {}

        void accept(AcyclicVisitor& v) override {
            auto* v1 = dynamic_cast<Visitor<CappedFlooredIborCoupon>*>(&v);
            if (v1 != nullptr)
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
                  const ext::shared_ptr<SwapIndex>& index,
                  Real gearing = 1.0,
                  Spread spread= 0.0,
                  const Rate cap = Null<Rate>(),
                  const Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false,
                  const Date& exCouponDate = Date())
        : CappedFlooredCoupon(ext::shared_ptr<FloatingRateCoupon>(new
            CmsCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                      index, gearing, spread, refPeriodStart, refPeriodEnd,
                      dayCounter, isInArrears, exCouponDate)), cap, floor) {}

        void accept(AcyclicVisitor& v) override {
            auto* v1 = dynamic_cast<Visitor<CappedFlooredCmsCoupon>*>(&v);
            if (v1 != nullptr)
                v1->visit(*this);
            else
                CappedFlooredCoupon::accept(v);
        }
    };

}

#endif


#ifndef id_2dc2464d480adebe2c4e09282927c6ae
#define id_2dc2464d480adebe2c4e09282927c6ae
inline bool test_2dc2464d480adebe2c4e09282927c6ae(const int* i) {
    return i != nullptr;
}
#endif
