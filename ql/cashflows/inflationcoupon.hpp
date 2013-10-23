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

/*! \file floatingratecoupon.hpp
 \brief Coupon paying a variable index-based rate
 */

#ifndef quantlib_inflation_coupon_hpp
#define quantlib_inflation_coupon_hpp

#include <ql/cashflows/coupon.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class InflationIndex;
    class YieldTermStructure;
    class InflationCouponPricer;

    //! Base inflation-coupon class
    /*! The day counter is usually obtained from the inflation term
        structure that the inflation index uses for forecasting.
        There is no gearing or spread because these are relevant for
        YoY coupons but not zero inflation coupons.

        \note inflation indices do not contain day counters or calendars.
    */
    class InflationCoupon : public Coupon,
                            public Observer {
    public:
        InflationCoupon(const Date& paymentDate,
                        Real nominal,
                        const Date& startDate,
                        const Date& endDate,
                        Natural fixingDays,
                        const boost::shared_ptr<InflationIndex>& index,
                        const Period& observationLag,
                        const DayCounter& dayCounter,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date()
                        );

        //! \name CashFlow interface
        //@{
        Real amount() const { return rate() * accrualPeriod() * nominal(); }
        //@}

        //! \name Coupon interface
        //@{
        Real price(const Handle<YieldTermStructure>& discountingCurve) const;
        DayCounter dayCounter() const { return dayCounter_; }
        Real accruedAmount(const Date&) const;
        Rate rate() const;
        //@}

        //! \name Inspectors
        //@{
        //! yoy inflation index
        const boost::shared_ptr<InflationIndex>& index() const { return index_; }
        //! how the coupon observes the index
        Period observationLag() const { return observationLag_; }
        //! fixing days
        Natural fixingDays() const { return fixingDays_; }
        //! fixing date
        virtual Date fixingDate() const;
        //! fixing of the underlying index, as observed by the coupon
        virtual Rate indexFixing() const;
        //@}

        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
        void setPricer(const boost::shared_ptr<InflationCouponPricer>&);
        boost::shared_ptr<InflationCouponPricer> pricer() const;

    protected:
        boost::shared_ptr<InflationCouponPricer> pricer_;
        boost::shared_ptr<InflationIndex> index_;
        Period observationLag_;
        DayCounter dayCounter_;
        Natural fixingDays_;

        //! makes sure you were given the correct type of pricer
        // this can also done in external pricer setter classes via
        // accept/visit mechanism
        virtual bool checkPricerImpl(const
            boost::shared_ptr<InflationCouponPricer>&) const = 0;
    };

    // inline definitions


    inline void InflationCoupon::accept(AcyclicVisitor& v) {
        Visitor<InflationCoupon>* v1 =
        dynamic_cast<Visitor<InflationCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }

    inline boost::shared_ptr<InflationCouponPricer>
    InflationCoupon::pricer() const {
        return pricer_;
    }

}

#endif
