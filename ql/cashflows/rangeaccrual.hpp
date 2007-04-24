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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rangeaccrual.hpp
*/

#ifndef quantlib_range_accrual_h
#define quantlib_range_accrual_h

#include <ql/termstructures/volatilities/smilesection.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/time/schedule.hpp>
#include <vector>

namespace QuantLib {

    class RangeAccrualPricer;

    class RangeAccrualFloatersCoupon: public IborCoupon{

      public:

          RangeAccrualFloatersCoupon(
                const Real nominal,
                const Date& paymentDate,
                const boost::shared_ptr<InterestRateIndex>& index,
                const Date& startDate,
                const Date& endDate,
                Integer fixingDays,
                const DayCounter& dayCounter,
                Real gearing,
                Rate spread,
                const Date& refPeriodStart,
                const Date& refPeriodEnd,
                const boost::shared_ptr<Schedule>&  observationsSchedule,
                Real lowerTrigger,
                Real upperTrigger,
                const boost::shared_ptr<RangeAccrualPricer>& Pricer);

        Real startTime() const {return startTime_; }
        Real endTime() const {return endTime_; }
        Real lowerTrigger() const {return lowerTrigger_; }
        Real upperTrigger() const {return upperTrigger_; }
        Size observationsNo() const {return observationsNo_; }
        const std::vector<Date>& observationDates() const {return observationDates_; }
        const std::vector<Real>& observationTimes() const {return observationTimes_; }
        const boost::shared_ptr<Schedule> observationsSchedule() const {
                                                        return observationsSchedule_; }

        Real rate() const;
        Real price(const Handle<YieldTermStructure>& discountingCurve) const;
        Real priceWithoutOptionality(const Handle<YieldTermStructure>& discountingCurve) const;

      private:
        
        Real startTime_;                               // S
        Real endTime_;                                 // T

        const boost::shared_ptr<Schedule> observationsSchedule_;
        std::vector<Date> observationDates_;
        std::vector<Real> observationTimes_;
        Size observationsNo_;

        Real lowerTrigger_;
        Real upperTrigger_;

        const boost::shared_ptr<RangeAccrualPricer> pricer_;

     };

    class RangeAccrualPricer {
      public:
        virtual ~RangeAccrualPricer() {}
        virtual Real price() const = 0;
        virtual Real rate() const = 0;
        virtual void initialize(const RangeAccrualFloatersCoupon& coupon) = 0;
    };

    class RangeAccrualPricerByBgm: public RangeAccrualPricer {

     public:
        RangeAccrualPricerByBgm(
            Real correlation,
            const  boost::shared_ptr<SmileSection>& smilesOnExpiry,
            const  boost::shared_ptr<SmileSection>& smilesOnPayment,
            bool withSmile,
            bool byCallSpread);

        Real price() const;
        Real rate() const;

     protected:

        void initialize(const RangeAccrualFloatersCoupon& coupon);

        Real drift(Real U, Real lambdaS, Real lambdaT, Real correlation) const;
        Real derDriftDerLambdaS(Real U, Real lambdaS, Real lambdaT, Real correlation) const;
        Real derDriftDerLambdaT(Real U, Real lambdaS, Real lambdaT, Real correlation) const;

        Real lambda(Real U, Real lambdaS, Real lambdaT) const;
        Real derLambdaDerLambdaS(Real U, Real lambdaS, Real lambdaT) const;
        Real derLambdaDerLambdaT(Real U, Real lambdaS, Real lambdaT) const;

        std::vector<Real> driftsOverPeriod(Real U, Real lambdaS, Real lambdaT, Real correlation) const;
        std::vector<Real> lambdasOverPeriod(Real U, Real lambdaS, Real lambdaT) const;

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

        Real correlation_;                                // correlation between L(S) and L(T)
        bool withSmile_;
        bool byCallSpread_;

        boost::shared_ptr<SmileSection> smilesOnExpiry_;
        boost::shared_ptr<SmileSection> smilesOnPayment_;

    };

}

#endif
