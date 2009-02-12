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

/*! \file nominalyoyinflationcoupon.hpp
    \brief Spread coupon between nominal and YoY inflation.
*/

#ifndef quantlib_nominal_yoy_inflation_coupon_hpp
#define quantlib_nominal_yoy_inflation_coupon_hpp

#include <ql/cashflows/coupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    class InflationCappedCouponPricer;  // forward declaration

    //! spread coupon between nominal and YoY inflation
    /*! still somewhat abstract because you don't say what the payoff
        is - this is just an information carrier
    */
    class NominalYoYInflationCoupon : public Coupon,
                                      public Observer {
      public:
        NominalYoYInflationCoupon(
                         const Date &paymentDate,
                         const Real nominal,
                         const Date &startDate,
                         const Date &endDate,
                         const Natural nominalFixingDays,
                         const BusinessDayConvention nominalBdc,
                         const boost::shared_ptr<IborIndex> &nominalIndex,
                         const bool yoyIsRatio,
                         const Period &yoyLag,
                         const boost::shared_ptr<YoYInflationIndex> &yoyIndex,
                         const DayCounter &accrualDayCounter);

        virtual ~NominalYoYInflationCoupon(){}

        //! \name CashFlow interface
        //@{
        virtual Real amount() const;
        //@}

        //! \name Coupon interface
        //@{
        virtual Real rate() const;
        virtual Real price(
                    const Handle<YieldTermStructure>& discountingCurve) const;
        virtual DayCounter dayCounter() const;
        virtual Real accruedAmount(const Date&) const;
        //@}

        //! \name Inspectors
        //@{
        /*! This is the date on which the coupon is known, i.e. the
            max of the two underlying fixing dates.
        */
        virtual Date fixingDate() const;
        virtual Date nominalFixingDate() const;
        virtual Natural nominalFixingDays() const;
        virtual const boost::shared_ptr<IborIndex> nominalIndex() const;
        virtual Date yoyFixingDate() const;
        virtual Period yoyLag() const;
        const boost::shared_ptr<YoYInflationIndex> yoyIndex() const;
        //@}

        //! \name Observer interface
        //@{
        virtual void update();
        //@}

      protected:
        BusinessDayConvention nominalBdc_;
        Natural nominalFixingDays_;
        boost::shared_ptr<IborIndex> nominalIndex_;
        bool yoyIsRatio_;
        Period yoyLag_;
        boost::shared_ptr<YoYInflationIndex> yoyIndex_;
        DayCounter accrualDayCounter_;

        // no pricer because we haven't said what the payoff is!
        // esseantially an information store for an instrument
    };


    class InflationCappedCoupon : public NominalYoYInflationCoupon {
      public:
        InflationCappedCoupon(
                         const Real nominalSpread,
                         const Real capGearing,
                         const Real inflationSpread,
                         const Date &paymentDate,
                         const Real nominal,
                         const Date &startDate,
                         const Date &endDate,
                         const Natural nominalFixingDays,
                         const BusinessDayConvention nominalBdc,
                         const boost::shared_ptr<IborIndex> &nominalIndex,
                         const bool yoyIsRatio,
                         const Period &yoyLag,
                         const boost::shared_ptr<YoYInflationIndex> &yoyIndex,
                         const DayCounter &accrualDayCounter);

        virtual ~InflationCappedCoupon() {}

        void setPricer(const boost::shared_ptr<InflationCappedCouponPricer>&);

        //! \name Inspectors
        //@{
        Real nominalSpread() const {return nominalSpread_;}
        Real capGearing() const {return capGearing_;}
        Real inflationSpread() const {return inflationSpread_;}
        //@}

        //! \name CashFlow interface
        //@{
        virtual Real amount() const;
        //@}

        //! \name Coupon interface
        //@{
        virtual Real rate() const;
        //@}


      protected:
        boost::shared_ptr<InflationCappedCouponPricer> pricer_;
        Real nominalSpread_;
        Real capGearing_;
        Real inflationSpread_;
    };

}

#endif

