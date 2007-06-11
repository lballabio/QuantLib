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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/interestrateindex.hpp>

#include <cmath>
#include <iostream>
#include <iomanip>

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
                Real lowerTrigger,                                    // l
                Real upperTrigger                                     // u
        )
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                        fixingDays, index, gearing, spread,
                        refPeriodStart, refPeriodEnd, dayCounter),
    observationsSchedule_(observationsSchedule),
    lowerTrigger_(lowerTrigger),
    upperTrigger_(upperTrigger){

        QL_REQUIRE(lowerTrigger_<upperTrigger,
            "RangeAccrualFloatersCoupon::RangeAccrualFloatersCoupon: lowerTrigger_>=upperTrigger");
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
        for(Size i=0;i<observationsNo_;i++) {
            observationTimes_.push_back(dayCounter.yearFraction(referenceDate, observationDates_[i]));
        }

     }

    void RangeAccrualFloatersCoupon::accept(AcyclicVisitor& v) {
        Visitor<RangeAccrualFloatersCoupon>* v1 =
            dynamic_cast<Visitor<RangeAccrualFloatersCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    Real RangeAccrualFloatersCoupon::priceWithoutOptionality(
           const Handle<YieldTermStructure>& discountingCurve) const {
        return accrualPeriod() * (gearing_*indexFixing()+spread_) *
               nominal() * discountingCurve->discount(date());
    }


    //===========================================================================//
    //                          RangeAccrualPricer                               //
    //===========================================================================//

    void RangeAccrualPricer::initialize(const FloatingRateCoupon& coupon){
        coupon_ =  dynamic_cast<const RangeAccrualFloatersCoupon*>(&coupon);
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();

        Date paymentDate = coupon_->date();

        const boost::shared_ptr<InterestRateIndex>& index = coupon_->index();
        const Handle<YieldTermStructure>& rateCurve = index->termStructure();
        discount_ = rateCurve->discount(paymentDate);
        accrualFactor_ = coupon_->accrualPeriod();
        spreadLegValue_ = spread_ * accrualFactor_* discount_;

        startTime_ = coupon_->startTime();
        endTime_ = coupon_->endTime();
        observationTimes_ = coupon_->observationTimes();
        lowerTrigger_ = coupon_->lowerTrigger();
        upperTrigger_ = coupon_->upperTrigger();
        observationsNo_ = coupon_->observationsNo();

        const std::vector<Date> &observationDates = coupon_->observationsSchedule()->dates();
        QL_REQUIRE(observationDates.size()==observationsNo_+2, "incompatible size of initialValues vector");
        initialValues_= std::vector<Real>(observationDates.size(),0.);

        Calendar calendar = index->fixingCalendar();
        for(Size i=0; i<observationDates.size(); i++) {
            initialValues_[i]=index->fixing(
                calendar.advance(observationDates[i],
                                 -static_cast<Integer>(coupon_->fixingDays()),
                                 Days));
        }

      }

    Real RangeAccrualPricer::swapletRate() const {
        return swapletPrice()/(accrualFactor_*discount_);
    }
    Real RangeAccrualPricer::capletPrice(Rate effectiveCap) const {
        QL_FAIL("RangeAccrualPricer::capletPrice not implemented");
    }
    Rate RangeAccrualPricer::capletRate(Rate effectiveCap) const {
        QL_FAIL("RangeAccrualPricer::capletRate not implemented");
    }
    Real RangeAccrualPricer::floorletPrice(Rate effectiveFloor) const {
        QL_FAIL("RangeAccrualPricer::floorletPrice not implemented");
    }
    Rate RangeAccrualPricer::floorletRate(Rate effectiveFloor) const {
        QL_FAIL("RangeAccrualPricer::floorletRate not implemented");
    }

    //===========================================================================//
    //                          RangeAccrualPricerByBgm                          //
    //===========================================================================//
    RangeAccrualPricerByBgm::RangeAccrualPricerByBgm(
            Real correlation,
            const  boost::shared_ptr<SmileSection>& smilesOnExpiry,
            const  boost::shared_ptr<SmileSection>& smilesOnPayment,
            bool withSmile,
            bool byCallSpread)
    : correlation_(correlation),
      withSmile_(withSmile),
      byCallSpread_(byCallSpread),
      smilesOnExpiry_(smilesOnExpiry),
      smilesOnPayment_(smilesOnPayment),
      eps_(1.0e-8) {

    }
    Real RangeAccrualPricerByBgm::swapletPrice() const{

        Real result = 0.;
        const Real deflator = discount_*initialValues_[0];
        for(Size i=0;i<observationsNo_;i++){
            Real digitalFloater = digitalRangePrice(lowerTrigger_, upperTrigger_,initialValues_[i+1],
                                                     observationTimes_[i], deflator);
            result += digitalFloater;
        }
        return gearing_ *(result*accrualFactor_/observationsNo_)+ spreadLegValue_;
    }

    std::vector<Real> RangeAccrualPricerByBgm::driftsOverPeriod(Real U,
                                                                Real lambdaS,
                                                                Real lambdaT,
                                                                Real correlation) const{
        std::vector<Real> result;

        const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;
        const Real L0T = initialValues_.back();

        const Real driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(p*lambdaT*lambdaT + q*lambdaS*lambdaT*correlation) +
                q*lambdaS*lambdaS + p*lambdaS*lambdaT*correlation
                -0.5*lambda(U,lambdaS,lambdaT)*lambda(U,lambdaS,lambdaT);
        const Real driftAfterFixing = (p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)-0.5)*lambdaT*lambdaT;

        result.push_back(driftBeforeFixing);
        result.push_back(driftAfterFixing);

        return result;
    }

    std::vector<Real> RangeAccrualPricerByBgm::lambdasOverPeriod(Real U,
                                                                   Real lambdaS,
                                                                   Real lambdaT) const{
        std::vector<Real> result;

        const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;

        const Real lambdaBeforeFixing = q*lambdaS + p*lambdaT;
        const Real lambdaAfterFixing = lambdaT;

        result.push_back(lambdaBeforeFixing);
        result.push_back(lambdaAfterFixing);

        return result;
    }
    Real RangeAccrualPricerByBgm::drift(Real U,
                                          Real lambdaS,
                                          Real lambdaT,
                                          Real correlation) const{
        Real result;

        const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;
        const Real L0T = initialValues_.back();

        const Real driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(p*lambdaT*lambdaT + q*lambdaS*lambdaT*correlation) +
                q*lambdaS*lambdaS + p*lambdaS*lambdaT*correlation;
        const Real driftAfterFixing = (p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)-0.5)*lambdaT*lambdaT;

        if(startTime_ > 0){result = driftBeforeFixing;}
        else {result = driftAfterFixing;}

        return result;
    }

    Real RangeAccrualPricerByBgm::lambda(Real U,
                                           Real lambdaS,
                                           Real lambdaT) const{
        Real result;

        const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;

        if(startTime_ > 0){result = q*lambdaS + p*lambdaT;}
        else {result = lambdaT;}

        return result;
    }


    Real RangeAccrualPricerByBgm::derDriftDerLambdaS(Real U,
                                                        Real lambdaS,
                                                        Real lambdaT,
                                                        Real correlation) const{
        Real result;

        const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;
        const Real L0T = initialValues_.back();

        const Real driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(q*lambdaT*correlation) +
                2*q*lambdaS + p*lambdaT*correlation;
        const Real driftAfterFixing = 0.;

        if(startTime_ > 0){result = driftBeforeFixing;}
        else {result = driftAfterFixing;}

        return result;
    }

    Real RangeAccrualPricerByBgm::derLambdaDerLambdaS(Real U,
                                                        Real lambdaS,
                                                        Real lambdaT) const{
        Real result;

        //const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;

        if(startTime_ > 0){result = q;}
        else {result = 0.;}

        return result;
    }

    Real RangeAccrualPricerByBgm::derDriftDerLambdaT(Real U,
                                                        Real lambdaS,
                                                        Real lambdaT,
                                                        Real correlation) const{
        Real result;

        const Real p = (U-startTime_)/accrualFactor_;
        const Real q = (endTime_-U)/accrualFactor_;
        const Real L0T = initialValues_.back();

        const Real driftBeforeFixing =
                p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)*(2*p*lambdaT + q*lambdaS*correlation) +
                + p*lambdaS*correlation;
        const Real driftAfterFixing = (p*accrualFactor_*L0T/(1.+L0T*accrualFactor_)-0.5)*2*lambdaT;

        if(startTime_ > 0){result = driftBeforeFixing;}
        else {result = driftAfterFixing;}

        return result;
    }

    Real RangeAccrualPricerByBgm::derLambdaDerLambdaT(Real U,
                                                        Real lambdaS,
                                                        Real lambdaT) const{
        Real result;

        const Real p = (U-startTime_)/accrualFactor_;
        //const Real q = (endTime_-U)/accrualFactor_;

        if(startTime_ > 0){result = p;}
        else {result = 1.;}

        return result;
    }

    Real RangeAccrualPricerByBgm::digitalRangePrice(Real lowerTrigger,
                                                      Real upperTrigger,
                                                      Real initialValue,
                                                      Real expiry,
                                                      Real deflator) const{
            const Real lowerPrice = digitalPrice(lowerTrigger, initialValue, expiry, deflator);
            const Real upperPrice = digitalPrice(upperTrigger, initialValue, expiry, deflator);
            const Real result =  lowerPrice - upperPrice;
            QL_REQUIRE(result >0.,
                "RangeAccrualPricerByBgm::digitalRangePrice:\n digitalPrice("<<upperTrigger<<
                "): "<<upperPrice<<" >  digitalPrice("<<lowerTrigger<<"): "<<lowerPrice);
            return result;

    }
    Real RangeAccrualPricerByBgm::digitalPrice(Real strike,
                                        Real initialValue,
                                        Real expiry,
                                        Real deflator) const {
        Real result = deflator;
        if(strike>eps_/2){
            if(withSmile_)
                result = digitalPriceWithSmile(strike, initialValue, expiry, deflator);
            else
                result = digitalPriceWithoutSmile(strike, initialValue, expiry, deflator);
        }
        //std::cout<<strike<<"\t"<<result<<"\n";
        return result;
    }

    Real RangeAccrualPricerByBgm::digitalPriceWithoutSmile(Real strike,
                                        Real initialValue,
                                        Real expiry,
                                        Real deflator) const {

        Real lambdaS = smilesOnExpiry_->volatility(strike);
        Real lambdaT = smilesOnPayment_->volatility(strike);

        std::vector<Real> lambdaU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
        const Real variance =
            startTime_*lambdaU[0]*lambdaU[0]+(expiry-startTime_)*lambdaU[1]*lambdaU[1];

        Real lambdaSATM = smilesOnExpiry_->volatility(initialValue);
        Real lambdaTATM = smilesOnPayment_->volatility(initialValue);
        //drift of Lognormal process (of Libor) "a_U()" nel paper
        std::vector<Real> muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);
        const Real adjustment = (startTime_*muU[0]+(expiry-startTime_)*muU[1]);


       Real d2 = (std::log(initialValue/strike) + adjustment - 0.5*variance)/std::sqrt(variance);

       CumulativeNormalDistribution phi;
       const Real result = deflator*phi(d2);

       QL_REQUIRE(result > 0.,
           "RangeAccrualPricerByBgm::digitalPriceWithoutSmile: result< 0. Result:"<<result);
       QL_REQUIRE(result/deflator <= 1.,
            "RangeAccrualPricerByBgm::digitalPriceWithoutSmile: result/deflator > 1. Ratio: "
            << result/deflator << " result: " << result<< " deflator: " << deflator);

        return result;
    }

    Real RangeAccrualPricerByBgm::digitalPriceWithSmile(Real strike,
                                        Real initialValue,
                                        Real expiry,
                                        Real deflator) const {
        Real result;
        if(byCallSpread_){

            // Previous strike
            const Real previousStrike = strike - eps_/2;
            Real lambdaS = smilesOnExpiry_->volatility(previousStrike);
            Real lambdaT = smilesOnPayment_->volatility(previousStrike);

            //drift of Lognormal process (of Libor) "a_U()" nel paper
            std::vector<Real> lambdaU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
            const Real previousVariance = std::max(startTime_, 0.)*lambdaU[0]*lambdaU[0]+
                         std::min(expiry-startTime_, expiry)*lambdaU[1]*lambdaU[1];

            Real lambdaSATM = smilesOnExpiry_->volatility(initialValue);
            Real lambdaTATM = smilesOnPayment_->volatility(initialValue);
            std::vector<Real> muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);
            const Real previousAdjustment = std::exp(std::max(startTime_, 0.)*muU[0] +
                                         std::min(expiry-startTime_, expiry)*muU[1]);
            const Real previousForward = initialValue * previousAdjustment ;

            // Next strike
            const Real nextStrike = strike + eps_/2;
            lambdaS = smilesOnExpiry_->volatility(nextStrike);
            lambdaT = smilesOnPayment_->volatility(nextStrike);

            lambdaU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
            const Real nextVariance = std::max(startTime_, 0.)*lambdaU[0]*lambdaU[0]+
                         std::min(expiry-startTime_, expiry)*lambdaU[1]*lambdaU[1];
            //drift of Lognormal process (of Libor) "a_U()" nel paper
            muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);
            const Real nextAdjustment = std::exp(std::max(startTime_, 0.)*muU[0] +
                                         std::min(expiry-startTime_, expiry)*muU[1]);
            const Real nextForward = initialValue * nextAdjustment ;

            result = callSpreadPrice(previousForward,nextForward,previousStrike, nextStrike,
                                                    deflator, previousVariance, nextVariance);

        }
        else{
            result = digitalPriceWithoutSmile(strike, initialValue, expiry, deflator)+
                     smileCorrection(strike, initialValue, expiry, deflator);
        }

        QL_REQUIRE(result > -pow(eps_,.5),
            "RangeAccrualPricerByBgm::digitalPriceWithSmile: result< 0 Result:"<<result);
        QL_REQUIRE(result/deflator <=  1.0 + pow(eps_,.2),
            "RangeAccrualPricerByBgm::digitalPriceWithSmile: result/deflator > 1. Ratio: "
            << result/deflator << " result: " << result<< " deflator: " << deflator);

        return result;
    }

    Real RangeAccrualPricerByBgm::smileCorrection(Real strike,
                                        Real forward,
                                        Real expiry,
                                        Real deflator) const {

        const Real previousStrike = strike - eps_/2;
        const Real nextStrike = strike + eps_/2;

        const Real derSmileS = (smilesOnExpiry_->volatility(nextStrike)-
                                 smilesOnExpiry_->volatility(previousStrike))/eps_;
        const Real derSmileT = (smilesOnPayment_->volatility(nextStrike)-
                                 smilesOnPayment_->volatility(previousStrike))/eps_;

        Real lambdaS = smilesOnExpiry_->volatility(strike);
        Real lambdaT = smilesOnPayment_->volatility(strike);
        //Real lambdaU = lambda(expiry, lambdaS, lambdaT);

        Real derLambdaDerK = derLambdaDerLambdaS(expiry, lambdaS, lambdaT)*derSmileS +
                               derLambdaDerLambdaT(expiry, lambdaS, lambdaT)*derSmileT;
        //Real derDriftDerK = derDriftDerLambdaS(expiry, lambdaS, lambdaT, correlation_)*derSmileS +
        //                      derDriftDerLambdaT(expiry, lambdaS, lambdaT, correlation_)*derSmileT +
        //                      lambdaU * derLambdaDerK;

        Real lambdaSATM = smilesOnExpiry_->volatility(forward);
        Real lambdaTATM = smilesOnPayment_->volatility(forward);
        std::vector<Real> lambdasOverPeriodU = lambdasOverPeriod(expiry, lambdaS, lambdaT);
        //drift of Lognormal process (of Libor) "a_U()" nel paper
        std::vector<Real> muU = driftsOverPeriod(expiry, lambdaSATM, lambdaTATM, correlation_);

        const Real variance = std::max(startTime_, 0.)*lambdasOverPeriodU[0]*lambdasOverPeriodU[0] +
                       std::min(expiry-startTime_, expiry)*lambdasOverPeriodU[1]*lambdasOverPeriodU[1];

        const Real forwardAdjustment = std::exp(std::max(startTime_, 0.)*muU[0] +
                                         std::min(expiry-startTime_, expiry)*muU[1]);
        const Real forwardAdjusted = forward * forwardAdjustment;

        const Real d1 = (std::log(forwardAdjusted/strike)+0.5*variance)/std::sqrt(variance);

        const Real sqrtOfTimeToExpiry = (std::max(startTime_, 0.)*lambdasOverPeriodU[0] +
                                std::min(expiry-startTime_, expiry)*lambdasOverPeriodU[1])*
                                (1./std::sqrt(variance));

        CumulativeNormalDistribution phi;
        NormalDistribution psi;
        Real result = - forwardAdjusted*psi(d1)*sqrtOfTimeToExpiry*derLambdaDerK ;
                       // - forwardAdjusted*phi(d1)*expiry*derDriftDerK;

        result *= deflator;

        QL_REQUIRE(std::fabs(result/deflator) <= 1.0 + pow(eps_,.2),
            "RangeAccrualPricerByBgm::smileCorrection: abs(result/deflator) > 1. Ratio: "
            << result/deflator << " result: " << result<< " deflator: " << deflator);

        return result;
    }

    Real RangeAccrualPricerByBgm::callSpreadPrice(
                                            Real previousForward,
                                            Real nextForward,
                                            Real previousStrike,
                                            Real nextStrike,
                                            Real deflator,
                                            Real previousVariance,
                                            Real nextVariance) const{
         const Real nextCall =
            blackFormula(Option::Call, nextStrike, nextForward, std::sqrt(nextVariance), deflator);
         const Real previousCall =
            blackFormula(Option::Call, previousStrike, previousForward, std::sqrt(previousVariance), deflator);

         QL_ENSURE(nextCall <previousCall,"RangeAccrualPricerByBgm::callSpreadPrice: nextCall > previousCall"
            "\n nextCall: strike :" << nextStrike << "; variance: " << nextVariance <<
            " adjusted initial value " << nextForward <<
            "\n previousCall: strike :" << previousStrike << "; variance: " << previousVariance <<
            " adjusted initial value " << previousForward );

         const Real result = (previousCall-nextCall)/(nextStrike-previousStrike);

         return result;
    }

}
