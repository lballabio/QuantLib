/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2016 Stefano Fondi

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

/*! \file overnightindexedcoupon.hpp
    \brief coupon paying the compounded daily overnight rate
*/

#ifndef quantlib_overnight_indexed_coupon_hpp
#define quantlib_overnight_indexed_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/cashflows/couponpricer.hpp>

namespace QuantLib {

    //! overnight coupon
    /*! %Coupon paying the compounded interest due to daily overnight fixings. */
    class OvernightIndexedCoupon : public FloatingRateCoupon {
      public:
        OvernightIndexedCoupon(
                    const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                    Real gearing = 1.0,
                    Spread spread = 0.0,
                    const Date& refPeriodStart = Date(),
                    const Date& refPeriodEnd = Date(),
                    const DayCounter& dayCounter = DayCounter());
        //! \name Inspectors
        //@{
        //! fixing dates for the rates to be compounded
        const std::vector<Date>& fixingDates() const { return fixingDates_; }
        //! accrual (compounding) periods
        const std::vector<Time>& dt() const { return dt_; }
        //! fixings to be compounded
        const std::vector<Rate>& indexFixings() const;
        //! value dates for the rates to be compounded
        const std::vector<Date>& valueDates() const { return valueDates_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        //! the date when the coupon is fully determined
        Date fixingDate() const { return fixingDates_.back(); }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      private:
        std::vector<Date> valueDates_, fixingDates_;
        mutable std::vector<Rate> fixings_;
        Size n_;
        std::vector<Time> dt_;
    };


    //! helper class building a sequence of overnight coupons
    class OvernightLeg {
      public:
        OvernightLeg(const Schedule& schedule,
                     const boost::shared_ptr<OvernightIndex>& overnightIndex);
        OvernightLeg& withNotionals(Real notional);
        OvernightLeg& withNotionals(const std::vector<Real>& notionals);
        OvernightLeg& withPaymentDayCounter(const DayCounter&);
        OvernightLeg& withPaymentAdjustment(BusinessDayConvention);
        OvernightLeg& withGearings(Real gearing);
        OvernightLeg& withGearings(const std::vector<Real>& gearings);
        OvernightLeg& withSpreads(Spread spread);
        OvernightLeg& withSpreads(const std::vector<Spread>& spreads);
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<OvernightIndex> overnightIndex_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
    };

    /*! pricer for arithmetically averaged overnight indexed coupons
    Reference: Katsumi Takada 2011, Valuation of Arithmetically Average of
    Fed Funds Rates and Construction of the US Dollar Swap Yield Curve
    */
    class ArithmeticAveragedOvernightIndexedCouponPricer
                                         : public FloatingRateCouponPricer {
    public:
        ArithmeticAveragedOvernightIndexedCouponPricer(
            Real meanReversion = 0.03,
            Real volatility = 0.00, // NO convexity adjustment by default
            bool byApprox = false) // TRUE to use Katsumi Takada approximation
        : byApprox_(byApprox), mrs_(meanReversion), vol_(volatility) {}

        void initialize(const FloatingRateCoupon& coupon);
        Rate swapletRate() const;
        Real swapletPrice() const { QL_FAIL("swapletPrice not available"); }
        Real capletPrice(Rate) const { QL_FAIL("capletPrice not available"); }
        Rate capletRate(Rate) const { QL_FAIL("capletRate not available"); }
        Real floorletPrice(Rate) const { QL_FAIL("floorletPrice not available"); }
        Rate floorletRate(Rate) const { QL_FAIL("floorletRate not available"); }
    protected:
        Real convAdj1(Time ts, Time te) const;
        Real convAdj2(Time ts, Time te) const;
        const OvernightIndexedCoupon* coupon_;
        bool byApprox_;
        Real mrs_;
        Real vol_;

    };

}

#endif
