/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Toyin Akin
 Copyright (C) 2021 Marcin Rybacki

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

/*! \file subperiodcoupon.hpp
    \brief averaging coupons
*/

#ifndef quantlib_sub_period_coupon_hpp
#define quantlib_sub_period_coupon_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/rateaveraging.hpp>
#include <ql/time/schedule.hpp>
#include <vector>

namespace QuantLib {

    class IborIndex;

    //! multiple-reset coupon
    /*! %Coupon paying a rate calculated by compounding or averaging
        multiple fixings during its accrual period.
    */
    class SubPeriodsCoupon: public FloatingRateCoupon {
      public:
        /*! \param resetSchedule the schedule for the multiple resets. The first and last
                                 dates are also the start and end dates of the coupon.
                                 Each period specified by the schedule is the underlying
                                 period for one fixing; the corresponding fixing date is
                                 the passed number of fixing days before the start of
                                 the period.
            \param couponSpread  an optional spread added to the final coupon rate.
            \param rateSpread    an optional spread added to each of the underlying fixings.
            \param gearing       an optional multiplier for the final coupon rate.
        */
        SubPeriodsCoupon(const Date& paymentDate,
                         Real nominal,
                         const Schedule& resetSchedule,
                         Natural fixingDays,
                         const ext::shared_ptr<IborIndex>& index,
                         Real gearing = 1.0,
                         Rate couponSpread = 0.0,
                         Rate rateSpread = 0.0,
                         const Date& refPeriodStart = Date(),
                         const Date& refPeriodEnd = Date(),
                         const DayCounter& dayCounter = DayCounter(),
                         const Date& exCouponDate = Date());

        /*! \deprecated Use the other constructor.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use the other constructor")]]
        SubPeriodsCoupon(const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         Natural fixingDays,
                         const ext::shared_ptr<IborIndex>& index,
                         Real gearing = 1.0,
                         Rate couponSpread = 0.0,
                         Rate rateSpread = 0.0,
                         const Date& refPeriodStart = Date(),
                         const Date& refPeriodEnd = Date(),
                         const DayCounter& dayCounter = DayCounter(),
                         const Date& exCouponDate = Date());

        //! \name Inspectors
        //@{
        //! fixing dates for the rates to be compounded
        const std::vector<Date>& fixingDates() const { return fixingDates_; }
        //! accrual (compounding) periods
        const std::vector<Time>& dt() const { return dt_; }
        //! value dates for the rates to be compounded
        const std::vector<Date>& valueDates() const { return valueDates_; }
        //! rate spread
        Spread rateSpread() const { return rateSpread_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        //! the date when the coupon is fully determined
        Date fixingDate() const override { return fixingDates_.back(); }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        Date fixingDate(const Date& valueDate) const;

        std::vector<Date> valueDates_, fixingDates_;
        Size n_;
        std::vector<Time> dt_;
        Rate rateSpread_;
    };

    class SubPeriodsPricer: public FloatingRateCouponPricer {
      public:
        Rate swapletPrice() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        void initialize(const FloatingRateCoupon& coupon) override;

      protected:
        const SubPeriodsCoupon* coupon_;
        std::vector<Real> subPeriodFixings_;
    };

    class AveragingRatePricer: public SubPeriodsPricer {
      public:
        Real swapletRate() const override;
    };

    class CompoundingRatePricer: public SubPeriodsPricer {
      public:
        Real swapletRate() const override;
    };


    //! helper class building a sequence of multiple-reset coupons
    class MultipleResetsLeg {
      public:
        /*! \param fullResetSchedule the full schedule specifying reset periods for all coupons.
            \param index             the index whose fixings will be used; it should have the
                                     same tenor as the resets.
            \param resetsPerCoupon   the number of resets for each coupon; the number of periods
                                     in the schedule should be divided exactly by this number.
        */
        MultipleResetsLeg(Schedule fullResetSchedule,
                          ext::shared_ptr<IborIndex> index,
                          Size resetsPerCoupon);
        MultipleResetsLeg& withNotionals(Real notional);
        MultipleResetsLeg& withNotionals(const std::vector<Real>& notionals);
        MultipleResetsLeg& withPaymentDayCounter(const DayCounter&);
        MultipleResetsLeg& withPaymentAdjustment(BusinessDayConvention);
        MultipleResetsLeg& withPaymentCalendar(const Calendar&);
        MultipleResetsLeg& withPaymentLag(Integer lag);
        MultipleResetsLeg& withFixingDays(Natural fixingDays);
        MultipleResetsLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        MultipleResetsLeg& withGearings(Real gearing);
        MultipleResetsLeg& withGearings(const std::vector<Real>& gearings);
        MultipleResetsLeg& withCouponSpreads(Spread spread);
        MultipleResetsLeg& withCouponSpreads(const std::vector<Spread>& spreads);
        MultipleResetsLeg& withRateSpreads(Spread spread);
        MultipleResetsLeg& withRateSpreads(const std::vector<Spread>& spreads);
        MultipleResetsLeg& withExCouponPeriod(const Period&,
                                              const Calendar&,
                                              BusinessDayConvention,
                                              bool endOfMonth = false);
        MultipleResetsLeg& withAveragingMethod(RateAveraging::Type averagingMethod);
        operator Leg() const;

      private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        Size resetsPerCoupon_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_ = Following;
        Integer paymentLag_ = 0;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> couponSpreads_;
        std::vector<Spread> rateSpreads_;
        RateAveraging::Type averagingMethod_ = RateAveraging::Compound;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_ = Unadjusted;
        bool exCouponEndOfMonth_ = false;
    };


    /*! \deprecated Use MultipleResetsLeg instead.
                    Deprecated in version 1.37.
    */
    class [[deprecated("Use MultipleResetsLeg instead")]] SubPeriodsLeg {
      public:
        SubPeriodsLeg(Schedule schedule, ext::shared_ptr<IborIndex> index);
        SubPeriodsLeg& withNotionals(Real notional);
        SubPeriodsLeg& withNotionals(const std::vector<Real>& notionals);
        SubPeriodsLeg& withPaymentDayCounter(const DayCounter&);
        SubPeriodsLeg& withPaymentAdjustment(BusinessDayConvention);
        SubPeriodsLeg& withPaymentCalendar(const Calendar&);
        SubPeriodsLeg& withPaymentLag(Integer lag);
        SubPeriodsLeg& withFixingDays(Natural fixingDays);
        SubPeriodsLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        SubPeriodsLeg& withGearings(Real gearing);
        SubPeriodsLeg& withGearings(const std::vector<Real>& gearings);
        SubPeriodsLeg& withCouponSpreads(Spread spread);
        SubPeriodsLeg& withCouponSpreads(const std::vector<Spread>& spreads);
        SubPeriodsLeg& withRateSpreads(Spread spread);
        SubPeriodsLeg& withRateSpreads(const std::vector<Spread>& spreads);
        SubPeriodsLeg& withExCouponPeriod(const Period&,
                                          const Calendar&,
                                          BusinessDayConvention,
                                          bool endOfMonth = false);
        SubPeriodsLeg& withAveragingMethod(RateAveraging::Type averagingMethod);
        operator Leg() const;

      private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_ = Following;
        Integer paymentLag_ = 0;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> couponSpreads_;
        std::vector<Spread> rateSpreads_;
        RateAveraging::Type averagingMethod_ = RateAveraging::Compound;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_ = Unadjusted;
        bool exCouponEndOfMonth_ = false;
    };
}

#endif
