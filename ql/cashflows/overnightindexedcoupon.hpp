/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer
 Copyright (C) 2025 Paolo D'Elia

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

    class CompoundingOvernightIndexedCouponPricer;
    class CappedFlooredOvernightIndexedCouponPricer;

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
                    bool applyObservationShift = false,
					          bool includeSpread = false,
                    const Date& rateComputationStartDate = Date(),
                    const Date& rateComputationEndDate = Date());
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
        RateAveraging::Type averagingMethod() const { return averagingMethod_; }
        //! lockout days
        Natural lockoutDays() const { return lockoutDays_; }
        //! apply observation shift
        bool applyObservationShift() const { return applyObservationShift_; }
        //! include spread in compounding?
        bool includeSpread() const { return includeSpread_; }
        /*! effectiveSpread and effectiveIndexFixing are set such that
            coupon amount = notional * accrualPeriod * ( gearing * effectiveIndexFixing + effectiveSpread )
            notice that
              - gearing = 1 is required if includeSpread = true
              - effectiveSpread = spread() if includeSpread = false */
        Real effectiveSpread() const;
        Real effectiveIndexFixing() const;
        //! rate computation start date
        const Date& rateComputationStartDate() const { return rateComputationStartDate_; }
        //! rate computation end date
        const Date& rateComputationEndDate() const { return rateComputationEndDate_; }
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
        bool canApplyTelescopicFormula() const {
            return fixingDays_ == index_->fixingDays() ||
                (applyObservationShift_ && index_->fixingDays() == 0);
        }
        //@}
      private:
        std::vector<Date> valueDates_, interestDates_, fixingDates_;
        mutable std::vector<Rate> fixings_;
        Size n_;
        std::vector<Time> dt_;
        RateAveraging::Type averagingMethod_;
        Natural lockoutDays_;
        bool applyObservationShift_;
        bool includeSpread_;
        Date rateComputationStartDate_, rateComputationEndDate_;

        Rate averageRate(const Date& date) const;
    };

    //! capped floored overnight indexed coupon
    class CappedFlooredOvernightIndexedCoupon : public FloatingRateCoupon {
    public:
        /*! capped / floored compounded, backward-looking on coupon, local means that the daily rates are capped / floored
          while a global cap / floor is applied to the effective period rate */
        CappedFlooredOvernightIndexedCoupon(const ext::shared_ptr<OvernightIndexedCoupon>& underlying,
                                            Real cap = Null<Real>(),
                                            Real floor = Null<Real>(), 
                                            bool nakedOption = false,
                                            bool localCapFloor = false);

        //! \name Observer interface
        //@{
        void deepUpdate() override;
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        void alwaysForwardNotifications();
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const override;
        Rate convexityAdjustment() const override;
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Date fixingDate() const override { return underlying_->fixingDate(); }
        //@}
        //! cap
        Rate cap() const;
        //! floor
        Rate floor() const;
        //! effective cap of fixing
        Rate effectiveCap() const;
        //! effective floor of fixing
        Rate effectiveFloor() const;
        //! effective caplet volatility
        Real effectiveCapletVolatility() const;
        //! effective floorlet volatility
        Real effectiveFloorletVolatility() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&) override;

        bool isCapped() const { return cap_ != Null<Real>(); }
        bool isFloored() const { return floor_ != Null<Real>(); }

        ext::shared_ptr<OvernightIndexedCoupon> underlying() const { return underlying_; }
        bool nakedOption() const { return nakedOption_; }
        bool localCapFloor() const { return localCapFloor_; }
        bool includeSpread() const { return underlying_->includeSpread(); }
        //! averaging method
        RateAveraging::Type averagingMethod() const { return underlying_->averagingMethod(); }

    protected:
        ext::shared_ptr<OvernightIndexedCoupon> underlying_;
        Rate cap_, floor_;
        bool nakedOption_;
        bool localCapFloor_;
        mutable Real effectiveCapletVolatility_;
        mutable Real effectiveFloorletVolatility_;
    };

    //! helper class building a sequence of overnight coupons
    class OvernightLeg {
      public:
        OvernightLeg(const Schedule& schedule, const ext::shared_ptr<OvernightIndex>& overnightIndex);
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
        OvernightLeg& includeSpread(bool includeSpread);
        OvernightLeg& withLookback(const Period& lookback);
        OvernightLeg& withCaps(Rate cap);
        OvernightLeg& withCaps(const std::vector<Rate>& caps);
        OvernightLeg& withFloors(Rate floor);
        OvernightLeg& withFloors(const std::vector<Rate>& floors);
        OvernightLeg& withNakedOption(const bool nakedOption);
        OvernightLeg& withLocalCapFloor(const bool localCapFloor);
        OvernightLeg& withInArrears(const bool inArrears);
        OvernightLeg& withLastRecentPeriod(const ext::optional<Period>& lastRecentPeriod);
        OvernightLeg& withLastRecentPeriodCalendar(const Calendar& lastRecentPeriodCalendar);
        OvernightLeg& withPaymentDates(const std::vector<Date>& paymentDates);
        OvernightLeg& withOvernightIndexedCouponPricer(const ext::shared_ptr<FloatingRateCouponPricer>& couponPricer);
        OvernightLeg& withCapFlooredOvernightIndexedCouponPricer(
          const QuantLib::ext::shared_ptr<CappedFlooredOvernightIndexedCouponPricer>& couponPricer);

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
        bool includeSpread_ = false;
        std::vector<Rate> caps_, floors_;
        bool nakedOption_;
        bool localCapFloor_;
        bool inArrears_ = true;
        ext::optional<Period> lastRecentPeriod_;
        Calendar lastRecentPeriodCalendar_;
        std::vector<Date> paymentDates_;
        ext::shared_ptr<FloatingRateCouponPricer> couponPricer_;
        ext::shared_ptr<CappedFlooredOvernightIndexedCouponPricer> capFlooredCouponPricer_;
    };

}

#endif
