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


#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <cmath>

namespace QuantLib {

    //===========================================================================//
    //                         RangeAccrualFloatersCoupon                        //
    //===========================================================================//

    RangeAccrualFloatersCoupon::RangeAccrualFloatersCoupon(
                const Real nominal,
                const Date& paymentDate,
                const boost::shared_ptr<InterestRateIndex>& index,
                const Date& startDate,                                  // S
                const Date& endDate,                                    // T
                Integer fixingDays,
                const DayCounter& dayCounter,
                Real gearing,
                Rate spread,
                const Date& refPeriodStart,
                const Date& refPeriodEnd,
                const boost::shared_ptr<Schedule>&  observationsSchedule,
                double lowerTrigger,                                    // l
                double upperTrigger,                                    // u
                const boost::shared_ptr<RangeAccrualPricer>& pricer
        )
    : IborCoupon(paymentDate, nominal, startDate, endDate,
                        fixingDays, index, gearing, spread,
                        refPeriodStart, refPeriodEnd, dayCounter),
    observationsSchedule_(observationsSchedule),
    lowerTrigger_(lowerTrigger),
    upperTrigger_(upperTrigger),
    pricer_(pricer){

        QL_REQUIRE(observationsSchedule_->startDate()==startDate,
            "RangeAccrualFloatersCoupon::RangeAccrualFloatersCoupon: incompatible start date");
        QL_REQUIRE(observationsSchedule_->endDate()==endDate,
            "RangeAccrualFloatersCoupon::RangeAccrualFloatersCoupon: incompatible end date");

        observationDates_ = observationsSchedule_->dates();
        observationDates_.pop_back();                           //remove end date
        observationDates_.erase(observationDates_.begin());     //remove start date
        observationsNo_ = observationDates_.size();

        const Handle<YieldTermStructure>& rateCurve = index->termStructure();
        const Date referenceDate = rateCurve->referenceDate();

        startTime_ = dayCounter.yearFraction(referenceDate, startDate);
        endTime_ = dayCounter.yearFraction(referenceDate, endDate);
        for(int i=0;i<observationsNo_;i++) {
            observationTimes_.push_back(dayCounter.yearFraction(referenceDate, observationDates_[i]));
        }

     }

    double RangeAccrualFloatersCoupon::rate() const {
            pricer_->initialize(*this);
            return gearing_*pricer_->rate()+spread_;
    }
    double RangeAccrualFloatersCoupon::price(const Handle<YieldTermStructure>& discountingCurve) const {
        return rate()*accrualPeriod()* nominal()*discountingCurve->discount(date());
    }
    double RangeAccrualFloatersCoupon::priceWithoutOptionality(const Handle<YieldTermStructure>& discountingCurve) const {
        return accrualPeriod() * (gearing_*indexFixing()+spread_) *
               nominal() * discountingCurve->discount(date());
    }

    //===========================================================================//
    //                          RangeAccrualPricerByBgm                          //
    //===========================================================================//
    RangeAccrualPricerByBgm::RangeAccrualPricerByBgm(
            double correlation,
            const  boost::shared_ptr<SmileSection>& smilesOnExpiry,
            const  boost::shared_ptr<SmileSection>& smilesOnPayment,
            bool withSmile,
            bool byCallSpread)
    :correlation_(correlation),
    smilesOnExpiry_(smilesOnExpiry),
    smilesOnPayment_(smilesOnPayment),
    withSmile_(withSmile),
    byCallSpread_(byCallSpread){

    }

    void RangeAccrualPricerByBgm::initialize(const RangeAccrualFloatersCoupon& coupon){
        Date paymentDate = coupon.date();

        const boost::shared_ptr<InterestRateIndex>& index = coupon.index();
        const Handle<YieldTermStructure>& rateCurve = index->termStructure();
        discount_ = rateCurve->discount(paymentDate);

        startTime_ = coupon.startTime_;
        endTime_ = coupon.endTime_;
        accrualFactor_ = coupon.accrualPeriod();
        observationTimes_ = coupon.observationTimes_;
        lowerTrigger_ = coupon.lowerTrigger_;
        upperTrigger_ = coupon.upperTrigger_;
        observationsNo_ = coupon.observationsNo_;

        const std::vector<Date> &observationDates = coupon.observationsSchedule_->dates();
        QL_REQUIRE(observationDates.size()==observationsNo_+2, "incompatible size of initialValues vector");
        initialValues_= std::vector<double>(observationDates.size(),0.);

        Calendar calendar = index->calendar();
        for(Size i=0; i<observationDates.size(); i++) {
            initialValues_[i]=index->fixing(
                calendar.advance(observationDates[i],
                                 -static_cast<Integer>(coupon.fixingDays()),
                                 Days));
        }

      }

    std::vector<double> RangeAccrualPricerByBgm::driftsOverPeriod(double U,
                                                                double lambdaS,
                                                                double lambdaT,
                                                                double correlation) const{
        std::vector<double> result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;
        const double L0T = initialValues_.back();

        const double driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(p*lambdaT*lambdaT + q*lambdaS*lambdaT*correlation) +
                q*lambdaS*lambdaS + p*lambdaS*lambdaT*correlation
                -0.5*lambda(U,lambdaS,lambdaT)*lambda(U,lambdaS,lambdaT);
        const double driftAfterFixing = (p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)-0.5)*lambdaT*lambdaT;

        result.push_back(driftBeforeFixing);
        result.push_back(driftAfterFixing);

        return result;
    }

    std::vector<double> RangeAccrualPricerByBgm::lambdasOverPeriod(double U,
                                                                   double lambdaS,
                                                                   double lambdaT) const{
        std::vector<double> result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;

        const double lambdaBeforeFixing = q*lambdaS + p*lambdaT;
        const double lambdaAfterFixing = lambdaT;

        result.push_back(lambdaBeforeFixing);
        result.push_back(lambdaAfterFixing);

        return result;
    }
    double RangeAccrualPricerByBgm::drift(double U,
                                          double lambdaS,
                                          double lambdaT,
                                          double correlation) const{
        double result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;
        const double L0T = initialValues_.back();

        const double driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(p*lambdaT*lambdaT + q*lambdaS*lambdaT*correlation) +
                q*lambdaS*lambdaS + p*lambdaS*lambdaT*correlation;
        const double driftAfterFixing = (p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)-0.5)*lambdaT*lambdaT;

        if(startTime_ > 0){result = driftBeforeFixing;}
        else {result = driftAfterFixing;}

        return result;
    }

    double RangeAccrualPricerByBgm::lambda(double U,
                                           double lambdaS,
                                           double lambdaT) const{
        double result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;

        if(startTime_ > 0){result = q*lambdaS + p*lambdaT;}
        else {result = lambdaT;}

        return result;
    }


    double RangeAccrualPricerByBgm::derDriftDerLambdaS(double U,
                                                        double lambdaS,
                                                        double lambdaT,
                                                        double correlation) const{
        double result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;
        const double L0T = initialValues_.back();

        const double driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(q*lambdaT*correlation) +
                2*q*lambdaS + p*lambdaT*correlation;
        const double driftAfterFixing = 0.;

        if(startTime_ > 0){result = driftBeforeFixing;}
        else {result = driftAfterFixing;}

        return result;
    }

    double RangeAccrualPricerByBgm::derLambdaDerLambdaS(double U,
                                                        double lambdaS,
                                                        double lambdaT) const{
        double result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;

        if(startTime_ > 0){result = q;}
        else {result = 0.;}

        return result;
    }

    double RangeAccrualPricerByBgm::derDriftDerLambdaT(double U,
                                                        double lambdaS,
                                                        double lambdaT,
                                                        double correlation) const{
        double result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;
        const double L0T = initialValues_.back();

        const double driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(2*p*lambdaT + q*lambdaS*correlation) +
                + p*lambdaS*correlation;
        const double driftAfterFixing = (p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)-0.5)*2*lambdaT;

        if(startTime_ > 0){result = driftBeforeFixing;}
        else {result = driftAfterFixing;}

        return result;
    }

    double RangeAccrualPricerByBgm::derLambdaDerLambdaT(double U,
                                                        double lambdaS,
                                                        double lambdaT) const{
        double result;

        const double p = (U-startTime_)/accrualFactor_;
        const double q = (endTime_-U)/accrualFactor_;

        if(startTime_ > 0){result = p;}
        else {result = 1.;}

        return result;
    }

    double RangeAccrualPricerByBgm::price() const{

        double result = 0.;
        const double deflator = discount_*initialValues_[0];

        for(int i=0;i<observationsNo_;i++){
            double digitalFloater = digitalRangePrice(lowerTrigger_, upperTrigger_,initialValues_[i+1],
                                                     observationTimes_[i], deflator);
            result += digitalFloater;
        }

        return result*accrualFactor_/observationsNo_;
    }


    double RangeAccrualPricerByBgm::rate() const {
        return price()/(accrualFactor_*discount_);
    }

    double RangeAccrualPricerByBgm::digitalRangePrice(double lowerTrigger,
                                                      double upperTrigger,
                                                      double initialValue,
                                                      double expiry,
                                                      double deflator) const{
            const double result =  digitalPrice(lowerTrigger, initialValue, expiry, deflator)-
                                   digitalPrice(upperTrigger, initialValue, expiry, deflator);
            QL_REQUIRE(result >0.,
                "RangeAccrualPricerByBgm::digitalRangePrice: digitalPrice(upper) >  digitalPrice(lower)");
            return result;

    }
    double RangeAccrualPricerByBgm::digitalPrice(double strike,
                                        double initialValue,
                                        double expiry,
                                        double deflator) const {
        double result;
        if(withSmile_)
            result = digitalPriceWithSmile(strike, initialValue, expiry, deflator);
        else
            result = digitalPriceWithoutSmile(strike, initialValue, expiry, deflator);
        return result;
    }

    double RangeAccrualPricerByBgm::digitalPriceWithoutSmile(double strike,
                                        double initialValue,
                                        double expiry,
                                        double deflator) const {

        double lambdaS = smilesOnExpiry_->volatility(strike);
        double lambdaT = smilesOnPayment_->volatility(strike);

        std::vector<double> lambdaU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
        const double variance =
            startTime_*lambdaU[0]*lambdaU[0]+(expiry-startTime_)*lambdaU[1]*lambdaU[1];

        double lambdaSATM = smilesOnExpiry_->volatility(initialValue);
        double lambdaTATM = smilesOnPayment_->volatility(initialValue);
        //drift of Lognormal process (of Libor) "a_U()" nel paper
        std::vector<double> muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);
        const double adjustment = (startTime_*muU[0]+(expiry-startTime_)*muU[1]);


       double d2 = (std::log(initialValue/strike) + adjustment - 0.5*variance)/std::sqrt(variance);

       CumulativeNormalDistribution phi;
       const double result = deflator*phi(d2);

      QL_REQUIRE(result > 0.,
            "RangeAccrualPricerByBgm::digitalPriceWithoutSmile: result< 0.");
       QL_REQUIRE(result/deflator <= 1.,
            "RangeAccrualPricerByBgm::digitalPriceWithoutSmile: result/deflator > 1. Ratio: "
            << result/deflator << " result: " << result<< " deflator: " << deflator);

        return result;
    }

    double RangeAccrualPricerByBgm::digitalPriceWithSmile(double strike,
                                        double initialValue,
                                        double expiry,
                                        double deflator) const {
        double result;
        if(byCallSpread_){

            const double eps = std::min(strike*1e-1, 1e-10);

            // Previous strike
            const double previousStrike = strike - eps/2;
            double lambdaS = smilesOnExpiry_->volatility(previousStrike);
            double lambdaT = smilesOnPayment_->volatility(previousStrike);

            //drift of Lognormal process (of Libor) "a_U()" nel paper
            std::vector<double> lambdaU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
            const double previousVariance = std::max(startTime_, 0.)*lambdaU[0]*lambdaU[0]+
                         std::min(expiry-startTime_, expiry)*lambdaU[1]*lambdaU[1];

            double lambdaSATM = smilesOnExpiry_->volatility(initialValue);
            double lambdaTATM = smilesOnPayment_->volatility(initialValue);
            std::vector<double> muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);
            const double previousAdjustment = std::exp(std::max(startTime_, 0.)*muU[0] +
                                         std::min(expiry-startTime_, expiry)*muU[1]);
            const double previousForward = initialValue * previousAdjustment ;

            // Next strike
            const double nextStrike = strike + eps/2;
            lambdaS = smilesOnExpiry_->volatility(nextStrike);
            lambdaT = smilesOnPayment_->volatility(nextStrike);

            lambdaU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
            const double nextVariance = std::max(startTime_, 0.)*lambdaU[0]*lambdaU[0]+
                         std::min(expiry-startTime_, expiry)*lambdaU[1]*lambdaU[1];
            //drift of Lognormal process (of Libor) "a_U()" nel paper
            muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);
            const double nextAdjustment = std::exp(std::max(startTime_, 0.)*muU[0] +
                                         std::min(expiry-startTime_, expiry)*muU[1]);
            const double nextForward = initialValue * nextAdjustment ;

            result = callSpreadPrice(previousForward,nextForward,previousStrike, nextStrike,
                                                    deflator, previousVariance, nextVariance);

        }
        else{
            result = digitalPriceWithoutSmile(strike, initialValue, expiry, deflator)+
                     smileCorrection(strike, initialValue, expiry, deflator);
        }

        QL_REQUIRE(result > 0.,
            "RangeAccrualPricerByBgm::digitalPriceWithSmile: result< 0.");
        QL_REQUIRE(result/deflator <=  1.000001,
            "RangeAccrualPricerByBgm::digitalPriceWithSmile: result/deflator > 1. Ratio: "
            << result/deflator << " result: " << result<< " deflator: " << deflator);

        return result;
    }

    double RangeAccrualPricerByBgm::smileCorrection(double strike,
                                        double forward,
                                        double expiry,
                                        double deflator) const {

        const double eps = std::min(strike*1e-1, 1e-10);
        const double previousStrike = strike - eps/2;
        const double nextStrike = strike + eps/2;

        const double derSmileS = (smilesOnExpiry_->volatility(nextStrike)-
                                 smilesOnExpiry_->volatility(previousStrike))/eps;
        const double derSmileT = (smilesOnPayment_->volatility(nextStrike)-
                                 smilesOnPayment_->volatility(previousStrike))/eps;

        double lambdaS = smilesOnExpiry_->volatility(strike);
        double lambdaT = smilesOnPayment_->volatility(strike);
        double lambdaU = lambda(expiry, lambdaS, lambdaT);

        double derLambdaDerK = derLambdaDerLambdaS(expiry, lambdaS, lambdaT)*derSmileS +
                               derLambdaDerLambdaT(expiry, lambdaS, lambdaT)*derSmileT;
        double derDriftDerK = derDriftDerLambdaS(expiry, lambdaS, lambdaT, correlation_)*derSmileS +
                              derDriftDerLambdaT(expiry, lambdaS, lambdaT, correlation_)*derSmileT +
                              lambdaU * derLambdaDerK;

        double lambdaSATM = smilesOnExpiry_->volatility(forward);
        double lambdaTATM = smilesOnPayment_->volatility(forward);
        std::vector<double> lambdasOverPeriodU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
        //drift of Lognormal process (of Libor) "a_U()" nel paper
        std::vector<double> muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);

        const double variance = std::max(startTime_, 0.)*lambdasOverPeriodU[0]*lambdasOverPeriodU[0] +
                       std::min(expiry-startTime_, expiry)*lambdasOverPeriodU[1]*lambdasOverPeriodU[1];

        const double forwardAdjustment = std::exp(std::max(startTime_, 0.)*muU[0] +
                                         std::min(expiry-startTime_, expiry)*muU[1]);
        const double forwardAdjusted = forward * forwardAdjustment;

        const double d1 = (std::log(forwardAdjusted/strike)+0.5*variance)/std::sqrt(variance);

        const double sqrtOfTimeToExpiry = (std::max(startTime_, 0.)*lambdasOverPeriodU[0] +
                                std::min(expiry-startTime_, expiry)*lambdasOverPeriodU[1])*
                                (1./std::sqrt(variance));

        CumulativeNormalDistribution phi;
        NormalDistribution psi;
        double result = - forwardAdjusted*psi(d1)*sqrtOfTimeToExpiry*derLambdaDerK ;
                       // - forwardAdjusted*phi(d1)*expiry*derDriftDerK;

        result *= deflator;

        QL_REQUIRE(abs(result/deflator) <= 1.000001,
            "RangeAccrualPricerByBgm::smileCorrection: abs(result/deflator) > 1. Ratio: "
            << result/deflator << " result: " << result<< " deflator: " << deflator);

        return result;
    }

    double RangeAccrualPricerByBgm::callSpreadPrice(
                                            double previousForward,
                                            double nextForward,
                                            double previousStrike,
                                            double nextStrike,
                                            double deflator,
                                            double previousVariance,
                                            double nextVariance) const{
         const double nextCall =
            blackFormula(Option::Call, nextStrike, nextForward, std::sqrt(nextVariance), deflator);
         const double previousCall =
            blackFormula(Option::Call, previousStrike, previousForward, std::sqrt(previousVariance), deflator);

         QL_ENSURE(nextCall <previousCall,"RangeAccrualPricerByBgm::callSpreadPrice: nextCall > previousCall"
            "\n nextCall: strike :" << nextStrike << "; variance: " << nextVariance <<
            " adjusted initial value " << nextForward <<
            "\n previousCall: strike :" << previousStrike << "; variance: " << previousVariance <<
            " adjusted initial value " << previousForward );

         const double result = (previousCall - nextCall)/(nextStrike-previousStrike);

         return result;
    }

}
