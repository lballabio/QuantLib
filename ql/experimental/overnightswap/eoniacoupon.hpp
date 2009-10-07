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

/*! \file overnightcoupon.hpp
    \brief coupon paying the compounded daily overnight rate
*/

#ifndef quantlib_overnight_coupon_hpp
#define quantlib_overnight_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! overnight coupon
    /*! %Coupon paying the compounded interest due to daily overnight fixings. */
    class OvernightIndexedCoupon : public FloatingRateCoupon {
      public:
        OvernightIndexedCoupon(const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const boost::shared_ptr<OvernightIndex>& index,
                    Real gearing = 1.0,
                    Spread spread = 0.0,
                    const Date& refPeriodStart = Date(),
                    const Date& refPeriodEnd = Date(),
                    const DayCounter& dayCounter = DayCounter());

        //! \name FloatingRateCoupon interface
        //@{
        //! not applicable here; use fixingDates() instead
        Date fixingDate() const;
        //! not applicable here; use indexFixings() instead
        Rate indexFixing() const;

        //! fixing dates of the rates to be averaged
        const std::vector<Date>& fixingDates() const {
            return valueDates_;
        }
        //! fixings of the underlying index to be averaged
        std::vector<Rate> indexFixings() const;
        //! fixings of the underlying index to be averaged
        const std::vector<Time>& dt() const { return dt_; }

        //! not applicable here
        Rate convexityAdjustment() const;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      private:
          std::vector<Date> valueDates_;
          Size n_;
          std::vector<Time> dt_;

    };


    //! helper class building a sequence of overnight coupons
    class OvernightLeg {
      public:
        OvernightLeg(const Schedule& schedule,
                 const boost::shared_ptr<OvernightIndex>& index);
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
        boost::shared_ptr<OvernightIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
    };

}


#endif
