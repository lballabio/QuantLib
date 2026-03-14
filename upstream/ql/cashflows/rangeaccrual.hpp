/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Copyright (C) 2006, 2007 Mario Pucci

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rangeaccrual.hpp
    \brief range-accrual coupon
*/

#ifndef quantlib_range_accrual_h
#define quantlib_range_accrual_h

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/time/schedule.hpp>
#include <vector>

namespace QuantLib {

    class IborIndex;

    class RangeAccrualFloatersCoupon : public FloatingRateCoupon {

      public:
        RangeAccrualFloatersCoupon(const Date& paymentDate,
                                   Real nominal,
                                   const ext::shared_ptr<IborIndex>& index,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Natural fixingDays,
                                   const DayCounter& dayCounter,
                                   Real gearing,
                                   Rate spread,
                                   const Date& refPeriodStart,
                                   const Date& refPeriodEnd,
                                   Schedule observationsSchedule,
                                   Real lowerTrigger,
                                   Real upperTrigger);

        /*! \deprecated Use the overload taking a Schedule instead.
                        Deprecated in version 1.40.
        */
        [[deprecated("Use the overload taking a Schedule instead")]]
        RangeAccrualFloatersCoupon(const Date& paymentDate,
                                   Real nominal,
                                   const ext::shared_ptr<IborIndex>& index,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Natural fixingDays,
                                   const DayCounter& dayCounter,
                                   Real gearing,
                                   Rate spread,
                                   const Date& refPeriodStart,
                                   const Date& refPeriodEnd,
                                   const ext::shared_ptr<Schedule>& observationsSchedule,
                                   Real lowerTrigger,
                                   Real upperTrigger)
        : RangeAccrualFloatersCoupon(paymentDate, nominal, index, startDate, endDate,
                                     fixingDays, dayCounter, gearing, spread,
                                     refPeriodStart, refPeriodEnd,
                                     *observationsSchedule,
                                     lowerTrigger, upperTrigger) {}

        Real startTime() const {return startTime_; }
        Real endTime() const {return endTime_; }
        Real lowerTrigger() const {return lowerTrigger_; }
        Real upperTrigger() const {return upperTrigger_; }
        Size observationsNo() const {return observationsNo_; }
        const std::vector<Date>& observationDates() const {
            return observationDates_;
        }
        const std::vector<Real>& observationTimes() const {
            return observationTimes_;
        }
        const Schedule& observationSchedule() const { return observationSchedule_; }
        /*! \deprecated Use observationSchedule instead.
                        Deprecated in version 1.40.
        */
        [[deprecated("Use observationSchedule instead")]]
        ext::shared_ptr<Schedule> observationsSchedule() const {
            return ext::make_shared<Schedule>(observationSchedule_);
        }

        Real priceWithoutOptionality(
                       const Handle<YieldTermStructure>& discountCurve) const;
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:

        Real startTime_;                               // S
        Real endTime_;                                 // T

        Schedule observationSchedule_;
        std::vector<Date> observationDates_;
        std::vector<Real> observationTimes_;
        Size observationsNo_;

        Real lowerTrigger_;
        Real upperTrigger_;
     };

    class RangeAccrualPricer: public FloatingRateCouponPricer {
      public:
        //! \name Observer interface
        //@{
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        void initialize(const FloatingRateCoupon& coupon) override;
        //@}

    protected:
        const RangeAccrualFloatersCoupon* coupon_;
        Real startTime_;                                   // S
        Real endTime_;                                     // T
        Real accrualFactor_;                               // T-S
        std::vector<Real> observationTimeLags_;            // d
        std::vector<Real> observationTimes_;               // U
        std::vector<Real> initialValues_;
        Size observationsNo_;
        Real lowerTrigger_;
        Real upperTrigger_;
        Real discount_;
        Real gearing_;
        Spread spread_;
        Real spreadLegValue_;

    };

    class RangeAccrualPricerByBgm : public RangeAccrualPricer {

     public:
       RangeAccrualPricerByBgm(Real correlation,
                               ext::shared_ptr<SmileSection> smilesOnExpiry,
                               ext::shared_ptr<SmileSection> smilesOnPayment,
                               bool withSmile,
                               bool byCallSpread);
       //! \name Observer interface
       //@{
       Real swapletPrice() const override;
       //@}

     protected:

        Real drift(Real U, Real lambdaS, Real lambdaT, Real correlation) const;
        Real derDriftDerLambdaS(Real U, Real lambdaS, Real lambdaT,
                                Real correlation) const;
        Real derDriftDerLambdaT(Real U, Real lambdaS, Real lambdaT,
                                Real correlation) const;

        Real lambda(Real U, Real lambdaS, Real lambdaT) const;
        Real derLambdaDerLambdaS(Real U) const;
        Real derLambdaDerLambdaT(Real U) const;

        std::vector<Real> driftsOverPeriod(Real U, Real lambdaS, Real lambdaT,
                                           Real correlation) const;
        std::vector<Real> lambdasOverPeriod(Real U, Real lambdaS,
                                            Real lambdaT) const;

        Real digitalRangePrice(Real lowerTrigger,
                                Real upperTrigger,
                                Real initialValue,
                                Real expiry,
                                Real deflator) const;

        Real digitalPrice(Real strike,
                    Real initialValue,
                    Real expiry,
                    Real deflator) const;

        Real digitalPriceWithoutSmile(Real strike,
                    Real initialValue,
                    Real expiry,
                    Real deflator) const;

        Real digitalPriceWithSmile(Real strike,
                    Real initialValue,
                    Real expiry,
                    Real deflator) const;

        Real callSpreadPrice(Real previousInitialValue,
                            Real nextInitialValue,
                            Real previousStrike,
                            Real nextStrike,
                            Real deflator,
                            Real previousVariance,
                            Real nextVariance) const;

        Real smileCorrection(Real strike,
                               Real initialValue,
                               Real expiry,
                               Real deflator) const;

     private:
        Real correlation_;   // correlation between L(S) and L(T)
        bool withSmile_;
        bool byCallSpread_;

        ext::shared_ptr<SmileSection> smilesOnExpiry_;
        ext::shared_ptr<SmileSection> smilesOnPayment_;
        Real eps_ = 1.0e-8;
    };


    //! helper class building a sequence of range-accrual floating-rate coupons
    class RangeAccrualLeg {
      public:
        RangeAccrualLeg(Schedule schedule, ext::shared_ptr<IborIndex> index);
        RangeAccrualLeg& withNotionals(Real notional);
        RangeAccrualLeg& withNotionals(const std::vector<Real>& notionals);
        RangeAccrualLeg& withPaymentDayCounter(const DayCounter&);
        RangeAccrualLeg& withPaymentAdjustment(BusinessDayConvention);
        RangeAccrualLeg& withFixingDays(Natural fixingDays);
        RangeAccrualLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        RangeAccrualLeg& withGearings(Real gearing);
        RangeAccrualLeg& withGearings(const std::vector<Real>& gearings);
        RangeAccrualLeg& withSpreads(Spread spread);
        RangeAccrualLeg& withSpreads(const std::vector<Spread>& spreads);
        RangeAccrualLeg& withLowerTriggers(Rate trigger);
        RangeAccrualLeg& withLowerTriggers(const std::vector<Rate>& triggers);
        RangeAccrualLeg& withUpperTriggers(Rate trigger);
        RangeAccrualLeg& withUpperTriggers(const std::vector<Rate>& triggers);
        RangeAccrualLeg& withObservationTenor(const Period&);
        RangeAccrualLeg& withObservationConvention(BusinessDayConvention);
        operator Leg() const;
      private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = Following;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> lowerTriggers_, upperTriggers_;
        Period observationTenor_;
        BusinessDayConvention observationConvention_ = ModifiedFollowing;
    };

}


#endif
