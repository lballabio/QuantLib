/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

/*! \file eoniacoupon.hpp
    \brief coupon paying the compounded daily Eonia rate
*/

#ifndef quantlib_eonia_coupon_hpp
#define quantlib_eonia_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Eonia coupon
    /*! %Coupon paying the compounded interest due to daily Eonia fixings. */
    class EoniaCoupon : public FloatingRateCoupon {
      public:
        EoniaCoupon(const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const boost::shared_ptr<Eonia>& index,
                    Real gearing = 1.0,
                    Spread spread = 0.0,
                    const Date& refPeriodStart = Date(),
                    const Date& refPeriodEnd = Date(),
                    const DayCounter& dayCounter = DayCounter());

        //! \name FloatingRateCoupon interface
        //@{
        //! not applicable here; use fixingDates() instead
        Date fixingDate() const;
        //! fixing dates of the rates to be averaged
        std::vector<Date> fixingDates() const;

        //! not applicable here; use indexFixings() instead
        Rate indexFixing() const;
        //! fixings of the underlying index to be averaged
        std::vector<Rate> indexFixings() const;

        //! not applicable here
        Rate convexityAdjustment() const;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      private:
        Schedule fixingSchedule_;
    };


    //! helper class building a sequence of Eonia coupons
    class EoniaLeg {
      public:
        EoniaLeg(const Schedule& schedule,
                 const boost::shared_ptr<Eonia>& index);
        EoniaLeg& withNotionals(Real notional);
        EoniaLeg& withNotionals(const std::vector<Real>& notionals);
        EoniaLeg& withPaymentDayCounter(const DayCounter&);
        EoniaLeg& withPaymentAdjustment(BusinessDayConvention);
        EoniaLeg& withGearings(Real gearing);
        EoniaLeg& withGearings(const std::vector<Real>& gearings);
        EoniaLeg& withSpreads(Spread spread);
        EoniaLeg& withSpreads(const std::vector<Spread>& spreads);
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<Eonia> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
    };

}


#endif
