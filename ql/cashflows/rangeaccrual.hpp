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

#include <ql/TermStructures/Volatilities/smilesection.hpp>
#include <ql/Indexes/iborindex.hpp>
#include <ql/CashFlows/all.hpp>
#include <vector>

namespace QuantLib {

    class RangeAccrualPricer;

    //! RangeAccrualFloatersCoupon
    /*!

    */
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
                double lowerTrigger,                                    
                double upperTrigger,
                const boost::shared_ptr<RangeAccrualPricer>& Pricer);

        double startTime_;                               // S
        double endTime_;                                 // T

        const boost::shared_ptr<Schedule> observationsSchedule_; 
        std::vector<Date> observationDates_;                    
        std::vector<double> observationTimes_;                  
        int observationsNo_;
        
        double lowerTrigger_;
        double upperTrigger_;

        double rate() const; 
        double price(const Handle<YieldTermStructure>& discountingCurve) const;
        double priceWithoutOptionality(const Handle<YieldTermStructure>& discountingCurve) const;

      private:

        const boost::shared_ptr<RangeAccrualPricer> pricer_;
 
     };
     
    //! VanillaRangeAccrualCouponPricer
    /*!

    */

    class RangeAccrualPricer {
      public:
        virtual ~RangeAccrualPricer() {}
        virtual Real price() const = 0;
        virtual Real rate() const = 0;
        virtual void initialize(const RangeAccrualFloatersCoupon& coupon) = 0;
    };
    
    //! RangeAccrualPricerByBgm
    /*!

    */ 
    class RangeAccrualPricerByBgm: public RangeAccrualPricer {

     public:
        RangeAccrualPricerByBgm(
            double correlation,
            const  boost::shared_ptr<SmileSection>& smilesOnExpiry,
            const  boost::shared_ptr<SmileSection>& smilesOnPayment,
            bool withSmile,
            bool byCallSpread);

        double price() const;
        double rate() const;

     protected:

        void initialize(const RangeAccrualFloatersCoupon& coupon);

        double drift(double U, double lambdaS, double lambdaT, double correlation) const;
        double derDriftDerLambdaS(double U, double lambdaS, double lambdaT, double correlation) const;
        double derDriftDerLambdaT(double U, double lambdaS, double lambdaT, double correlation) const;
        
        double lambda(double U, double lambdaS, double lambdaT) const;
        double derLambdaDerLambdaS(double U, double lambdaS, double lambdaT) const;
        double derLambdaDerLambdaT(double U, double lambdaS, double lambdaT) const;

        std::vector<double> driftsOverPeriod(double U, double lambdaS, double lambdaT, double correlation) const;
        std::vector<double> lambdasOverPeriod(double U, double lambdaS, double lambdaT) const;

        double digitalRangePrice(double lowerTrigger,
                                double upperTrigger,
                                double initialValue,
                                double expiry,
                                double deflator) const;

        double digitalPrice(double strike,
                    double initialValue,
                    double expiry,
                    double deflator) const;

        double digitalPriceWithoutSmile(double strike,
                    double initialValue,
                    double expiry,
                    double deflator) const;

        double digitalPriceWithSmile(double strike,
                    double initialValue,
                    double expiry,
                    double deflator) const;

        double callSpreadPrice(double previousInitialValue,
                            double nextInitialValue,
                            double previousStrike,
                            double nextStrike,
                            double deflator,
                            double previousVariance,
                            double nextVariance) const;

        double smileCorrection(double strike,
                               double initialValue,
                               double expiry,
                               double deflator) const;

     private:

        double startTime_;                                   // S
        double endTime_;                                     // T
        double accrualFactor_;                               // T-S
        std::vector<double> observationTimeLags_;            // d
        std::vector<double> observationTimes_;               // U
        std::vector<double> initialValues_;
        int observationsNo_;
        double lowerTrigger_;
        double upperTrigger_; 
        double discount_;
        
        double correlation_;                                // correlation between L(S) and L(T)
        bool withSmile_;
        bool byCallSpread_;

        boost::shared_ptr<SmileSection> smilesOnExpiry_;
        boost::shared_ptr<SmileSection> smilesOnPayment_;

    };

}

#endif
