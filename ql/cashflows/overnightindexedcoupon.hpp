/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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
#include <ql/cashflows/rateaveraging.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! overnight coupon
    /*! %Coupon paying the interest, depending on the averaging convention,
        due to daily overnight fixings.

        \warning telescopicValueDates optimizes the schedule for calculation speed,
        but might fail to produce correct results if the coupon ages by more than
        a grace period of 7 days. It is therefore recommended not to set this flag
        to true unless you know exactly what you are doing. The intended use is
        rather by the OISRateHelper which is safe, since it reinitialises the
        instrument each time the evaluation date changes.
    */
    class OvernightIndexedCoupon : public FloatingRateCoupon {
      public:
        OvernightIndexedCoupon(
                    const Date& paymentDate,
                    Real nominal,
                    const Date& startDate,
                    const Date& endDate,
                    const ext::shared_ptr<OvernightIndex>& overnightIndex,
                    Real gearing = 1.0,
                    Spread spread = 0.0,
                    const Date& refPeriodStart = Date(),
                    const Date& refPeriodEnd = Date(),
                    const DayCounter& dayCounter = DayCounter(),
                    bool telescopicValueDates = false,
                    RateAveraging::Type averagingMethod = RateAveraging::Compound);
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
        Date fixingDate() const override { return fixingDates_.back(); }
        Real accruedAmount(const Date&) const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        std::vector<Date> valueDates_, fixingDates_;
        mutable std::vector<Rate> fixings_;
        Size n_;
        std::vector<Time> dt_;

        Rate averageRate(const Date& date) const;
    };


    //! helper class building a sequence of overnight coupons
    class OvernightLeg {
      public:
        OvernightLeg(const Schedule& schedule, ext::shared_ptr<OvernightIndex> overnightIndex);
        OvernightLeg& withNotionals(Real notional);
        OvernightLeg& withNotionals(const std::vector<Real>& notionals);
        OvernightLeg& withPaymentDayCounter(const DayCounter&);
        OvernightLeg& withPaymentAdjustment(BusinessDayConvention);
        OvernightLeg& withPaymentCalendar(const Calendar&);
        OvernightLeg& withPaymentLag(Integer lag);
        OvernightLeg& withGearings(Real gearing);
        OvernightLeg& withGearings(const std::vector<Real>& gearings);
        OvernightLeg& withSpreads(Spread spread);
        OvernightLeg& withSpreads(const std::vector<Spread>& spreads);
        OvernightLeg& withTelescopicValueDates(bool telescopicValueDates);
        OvernightLeg& withAveragingMethod(RateAveraging::Type averagingMethod);
        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_ = Following;
        Integer paymentLag_ = 0;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        bool telescopicValueDates_ = false;
        RateAveraging::Type averagingMethod_ = RateAveraging::Compound;
    };

}

#endif
