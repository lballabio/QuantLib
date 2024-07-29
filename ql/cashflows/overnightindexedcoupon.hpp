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

#include <ql/cashflows/couponpricer.hpp>
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
                    RateAveraging::Type averagingMethod = RateAveraging::Compound,
                    Natural lookbackDays = Null<Natural>(),
                    Natural lockoutDays = 0,
                    bool applyObservationShift = false);
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
        //! interest dates for the rates to be compounded
        const std::vector<Date>& interestDates() const { return interestDates_; }
        //! averaging method
        const RateAveraging::Type averagingMethod() const { return averagingMethod_; }
        //! lockout days
        const Natural lockoutDays() const { return lockoutDays_; }
        //! apply observation shift
        const bool applyObservationShift() const { return applyObservationShift_; }
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
        //! \name Telescopic property
        //! Telescopic formula cannot be used with lookback days
        //! being different than intrinsic index fixing delay.
        //! Only when index fixing delay is 0 and observation shift is used,
        //! we can apply telescopic formula, when applying lookback period.
        //@{
        const bool canApplyTelescopicFormula() const;
        //@}
      private:
        std::vector<Date> valueDates_, interestDates_, fixingDates_;
        mutable std::vector<Rate> fixings_;
        Size n_;
        std::vector<Time> dt_;
        RateAveraging::Type averagingMethod_;
        Natural lockoutDays_;
        bool applyObservationShift_;

        Rate averageRate(const Date& date) const;
    };

    //! CompoudAveragedOvernightIndexedCouponPricer pricer
    class CompoudingOvernightIndexedCouponPricer : public FloatingRateCouponPricer {
      public:
        //! \name FloatingRateCoupon interface
        //@{
        void initialize(const FloatingRateCoupon& coupon) override;
        Rate swapletRate() const override;
        Real swapletPrice() const override { QL_FAIL("swapletPrice not available"); }
        Real capletPrice(Rate) const override { QL_FAIL("capletPrice not available"); }
        Rate capletRate(Rate) const override { QL_FAIL("capletRate not available"); }
        Real floorletPrice(Rate) const override { QL_FAIL("floorletPrice not available"); }
        Rate floorletRate(Rate) const override { QL_FAIL("floorletRate not available"); }
        //@}
        Rate averageRate(const Date& date) const;

      protected:
        const OvernightIndexedCoupon* coupon_ = nullptr;
    };

    //! helper class building a sequence of overnight coupons
    class OvernightLeg {
      public:
        OvernightLeg(Schedule schedule, ext::shared_ptr<OvernightIndex> overnightIndex);
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
        OvernightLeg& withLookbackDays(Natural lookbackDays);
        OvernightLeg& withLockoutDays(Natural lockoutDays);
        OvernightLeg& withObservationShift(bool applyObservationShift = true);
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
        Natural lookbackDays_ = Null<Natural>();
        Natural lockoutDays_ = 0;
        bool applyObservationShift_ = false;
    };

    inline const bool OvernightIndexedCoupon::canApplyTelescopicFormula() const {
        return fixingDays_ == index_->fixingDays() ||
               (applyObservationShift_ && index_->fixingDays() == 0);
    }

}

#endif
