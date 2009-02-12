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

#include <ql/experimental/inflation/inflationcoupon.hpp>

namespace QuantLib {

    //! Capped or floored inflation coupon.
    /*! Essentially a copy of the nominal version but taking a
        different index and a set of pricers (not just one).

        Note that we could have made CappedFlooredYoYInflationCoupon
        inherit from IC not YoYIC but in that case we'd have to use
        virtual inheritance to get around the diamond problem.  At
        this point the design is extra code rather than extra
        complexity.

        Note also that a caplet in usual terminology = (coupon_swaplet
        - coupon_caplet) because of change of point of view.  I.e. a
        capped/floored thing stays inside its boundaries whereas a
        usual caplet pays outside its boundary.  Hence we add a
        paysWithin parameter so that we can choose wether to have a
        coupon or an instrument point of view.

        The payoff \f$ P \f$ of a capped inflation-rate coupon
        with paysWithin = true is:
        \f[ P = N \times T \times \min(a L + b, C). \f]
        The payoff of a floored inflation-rate coupon is:
        \f[ P = N \times T \times \max(a L + b, F). \f]
        The payoff of a collared inflation-rate coupon is:
        \f[ P = N \times T \times \min(\max(a L + b, F), C). \f]
        If paysWithin = false then the inverse is returned
        (this provides for instrument cap and caplet prices).

        where \f$ N \f$ is the notional, \f$ T \f$ is the accrual
        time, \f$ L \f$ is the inflation rate, \f$ a \f$ is its
        gearing, \f$ b \f$ is the spread, and \f$ C \f$ and \f$ F \f$
        the strikes.

        They can be decomposed in the following manner.
        Decomposition of a capped floating rate coupon
        when paysWithin = true:
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
        CappedFlooredYoYInflationCoupon(
                      const boost::shared_ptr<YoYInflationCoupon>& underlying,
                      Rate cap = Null<Rate>(),
                      Rate floor = Null<Rate>(),
                      bool paysWithin = true);

        CappedFlooredYoYInflationCoupon(
                  const Date& paymentDate,
                  const Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const Period &lag,
                  const Natural fixingDays,
                  const boost::shared_ptr<YoYInflationIndex>& index,  // YoY
                  const Real gearing = 1.0,
                  const Spread spread = 0.0,
                  const DayCounter& dayCounter = Actual365Fixed(),
                  const BusinessDayConvention &bdc = ModifiedFollowing,
                  const Calendar &cal = TARGET(),
                  const bool paysWithin = true,
                  const Rate cap = Null<Rate>(),
                  const Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date())
        : YoYInflationCoupon(paymentDate, nominal, startDate, endDate,
                             lag, fixingDays, index, gearing, spread,
                             dayCounter, bdc, cal,
                             refPeriodStart, refPeriodEnd),
          paysWithin_(paysWithin), isFloored_(false), isCapped_(false) {
            underlying_ = boost::shared_ptr<YoYInflationCoupon>(
                                               new YoYInflationCoupon(*this));
            setCommon(cap, floor);
            registerWith(underlying_);  // its possible that some
                                        // component will change, e.g.
                                        // something in the index
        }

        //! \name augmented Coupon interface
        //@{
        //! swap(let) rate
        Rate rate() const;
        //! coupon or instrument perspective?
        bool paysWithin() const {return paysWithin_;}
        bool isCapped() const {return isCapped_;}
        bool isFloored() const {return isFloored_;}
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
        virtual void accept(AcyclicVisitor& v) {
            Visitor<CappedFlooredYoYInflationCoupon>* v1 =
                dynamic_cast<Visitor<CappedFlooredYoYInflationCoupon>*>(&v);
            if (v1 != 0)
                v1->visit(*this);
            else
                CappedFlooredYoYInflationCoupon::accept(v);
        }
        //@}

        void setPricer(const boost::shared_ptr<YoYInflationCouponPricer>&);

      protected:
        virtual void setCommon(const Rate &cap, const Rate &floor);
        // data
        boost::shared_ptr<YoYInflationCoupon> underlying_;
        bool paysWithin_, isFloored_, isCapped_;
        Rate cap_, floor_;
    };

}

#endif

