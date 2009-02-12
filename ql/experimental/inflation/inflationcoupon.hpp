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

/*! \file inflationcoupon.hpp
    \brief Coupons involving inflation
*/

#ifndef quantlib_inflation_coupon_hpp
#define quantlib_inflation_coupon_hpp

#include <ql/cashflows/coupon.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/option.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    class YoYInflationCouponPricer;

    //! base inflation coupon class
    class InflationCoupon : public Coupon,
                            public Observer {
      public:
        InflationCoupon(const Date& paymentDate,
                        const Real nominal,
                        const Date& startDate,
                        const Date& endDate,
                        const Period &lag,
                        const Natural fixingDays,
                        const Real gearing = 1.0,
                        const Spread spread = 0.0,
                        const DayCounter& dayCounter = Actual365Fixed(),
                        const BusinessDayConvention &bdc = ModifiedFollowing,
                        const Calendar &cal = TARGET(),
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date());
        // already in arrears and cannot be in advance
        // but can have lag different from index
        // ... any convexity adj for this is not yet
        // implemented

        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}

        //! \name Coupon interface
        //@{
        virtual Rate rate() const = 0;
        // we DO need to define this ... FloatingRateCoupon takes it
        // as the swaplet price
        // N.B. this is not the swaption rate !!
        // this calls pricer_->initialize(*this)
        // which has to make sure that it has the right sort of coupon
        Real cleanPrice(
                    const Handle<YieldTermStructure>& discountingCurve) const;
        DayCounter dayCounter() const { return dayCounter_; }
        BusinessDayConvention businessDayConvention() const {
            return businessDayConvention_;
        }
        Real accruedAmount(const Date&) const;
        //@}

        //! \name Inspectors
        //@{

        /*! fixing lag, may be the same as the index or not
            if not will induce a convexity correction
            \todo Add convexity correction
        */
        virtual Period lag() const {return lag_;}
        //! calendar is used by the reference period via the lag
        virtual Calendar calendar() const {return cal_;}
        //! fixing date
        virtual Date fixingDate() const;
        virtual Natural fixingDays() const { return fixingDays_; }
        //! index gearing, i.e. multiplicative coefficient for the index
        virtual Real gearing() const { return gearing_; }
        //! spread paid over the fixing of the underlying index
        virtual Spread spread() const { return spread_; }
        //! fixing of the underlying index
        virtual Rate indexFixing() const;
        //@}

        //! \name Observer interface
        //@{
        virtual void update() {notifyObservers();}
        //@}

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}

      protected:
        // back door to get the common feature of the index without
        // the specific features used later
        virtual boost::shared_ptr<InflationIndex> myindex() const = 0;

        DayCounter dayCounter_;
        BusinessDayConvention businessDayConvention_;
        Calendar cal_;
        Period lag_;
        Natural fixingDays_;
        Real gearing_;
        Spread spread_;
    };


    //! base inflation-coupon class
    class YoYInflationCoupon : public InflationCoupon {
      public:
        YoYInflationCoupon(const Date& paymentDate,
                           const Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           const Period &lag,
                           const Natural fixingDays,
                           const boost::shared_ptr<YoYInflationIndex>& index,
                           const Real gearing = 1.0,
                           const Spread spread = 0.0,
                           const DayCounter& dayCounter = Actual365Fixed(),
                           const BusinessDayConvention &bdc = ModifiedFollowing,
                           const Calendar &cal = TARGET(),
                           const Date& refPeriodStart = Date(),
                           const Date& refPeriodEnd = Date());
        // already in arrears and cannot be in advance

        //! inflation index
        virtual const boost::shared_ptr<YoYInflationIndex>& index() const {
            return index_;
        }
        virtual Rate rate() const;

        /*! There are many different ways of pricing inflation
            cap/floors, these are different interpretations of the
            volatility, which should have been derived the same way as
            the pricing.  E.g. don't interpret Black vols as Displaced
            Diffusion Black vols or vice versa.
        */
        virtual void setPricer(
                          const boost::shared_ptr<YoYInflationCouponPricer>&);
        virtual boost::shared_ptr<YoYInflationCouponPricer> pricer() const {
            return pricer_;
        }

      protected:

        virtual boost::shared_ptr<InflationIndex> myindex() const {
            return index();
        }

        boost::shared_ptr<YoYInflationCouponPricer> pricer_;
        boost::shared_ptr<YoYInflationIndex> index_;
    };

}

#endif

