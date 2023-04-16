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


#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    //===========================================================================//
    //                         RangeAccrualFloatersCoupon                        //
    //===========================================================================//

    RangeAccrualFloatersCoupon::RangeAccrualFloatersCoupon(
        const Date& paymentDate,
        Real nominal,
        const std::shared_ptr<IborIndex>& index,
        const Date& startDate, // S
        const Date& endDate,   // T
        Natural fixingDays,
        const DayCounter& dayCounter,
        Real gearing,
        Rate spread,
        const Date& refPeriodStart,
        const Date& refPeriodEnd,
        std::shared_ptr<Schedule> observationsSchedule,
        Real lowerTrigger, // l
        Real upperTrigger  // u
        )
    : FloatingRateCoupon(paymentDate,
                         nominal,
                         startDate,
                         endDate,
                         fixingDays,
                         index,
                         gearing,
                         spread,
                         refPeriodStart,
                         refPeriodEnd,
                         dayCounter),
      observationsSchedule_(std::move(observationsSchedule)), lowerTrigger_(lowerTrigger),
      upperTrigger_(upperTrigger) {

        QL_REQUIRE(lowerTrigger_<upperTrigger,
                   "lowerTrigger_>=upperTrigger");
        QL_REQUIRE(observationsSchedule_->startDate()==startDate,
                   "incompatible start date");
        QL_REQUIRE(observationsSchedule_->endDate()==endDate,
                   "incompatible end date");

        observationDates_ = observationsSchedule_->dates();
        observationDates_.pop_back();                       //remove end date
        observationDates_.erase(observationDates_.begin()); //remove start date
        observationsNo_ = observationDates_.size();

        const Handle<YieldTermStructure>& rateCurve =
            index->forwardingTermStructure();
        Date referenceDate = rateCurve->referenceDate();

        startTime_ = dayCounter.yearFraction(referenceDate, startDate);
        endTime_ = dayCounter.yearFraction(referenceDate, endDate);
        for(Size i=0;i<observationsNo_;i++) {
            observationTimes_.push_back(
                dayCounter.yearFraction(referenceDate, observationDates_[i]));
        }
    }

    void RangeAccrualFloatersCoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<RangeAccrualFloatersCoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    Real RangeAccrualFloatersCoupon::priceWithoutOptionality(
           const Handle<YieldTermStructure>& discountingCurve) const {
        return accrualPeriod() * (gearing_*indexFixing()+spread_) *
               nominal() * discountingCurve->discount(date());
    }


    //=======================================================================//
    //                        RangeAccrualPricer                             //
    //=======================================================================//

    void RangeAccrualPricer::initialize(const FloatingRateCoupon& coupon){
        coupon_ =  dynamic_cast<const RangeAccrualFloatersCoupon*>(&coupon);
        QL_REQUIRE(coupon_, "range-accrual coupon required");
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();

        Date paymentDate = coupon_->date();

        std::shared_ptr<IborIndex> index =
            std::dynamic_pointer_cast<IborIndex>(coupon_->index());
        const Handle<YieldTermStructure>& rateCurve =
            index->forwardingTermStructure();
        discount_ = rateCurve->discount(paymentDate);
        accrualFactor_ = coupon_->accrualPeriod();
        spreadLegValue_ = spread_ * accrualFactor_* discount_;

        startTime_ = coupon_->startTime();
        endTime_ = coupon_->endTime();
        observationTimes_ = coupon_->observationTimes();
        lowerTrigger_ = coupon_->lowerTrigger();
        upperTrigger_ = coupon_->upperTrigger();
        observationsNo_ = coupon_->observationsNo();

        const std::vector<Date> &observationDates =
            coupon_->observationsSchedule()->dates();
        QL_REQUIRE(observationDates.size()==observationsNo_+2,
                   "incompatible size of initialValues vector");
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

    Real RangeAccrualPricer::capletPrice(Rate) const {
        QL_FAIL("RangeAccrualPricer::capletPrice not implemented");
    }

    Rate RangeAccrualPricer::capletRate(Rate) const {
        QL_FAIL("RangeAccrualPricer::capletRate not implemented");
    }

    Real RangeAccrualPricer::floorletPrice(Rate) const {
        QL_FAIL("RangeAccrualPricer::floorletPrice not implemented");
    }

    Rate RangeAccrualPricer::floorletRate(Rate) const {
        QL_FAIL("RangeAccrualPricer::floorletRate not implemented");
    }

    //===========================================================================//
    //                          RangeAccrualPricerByBgm                          //
    //===========================================================================//
    RangeAccrualPricerByBgm::RangeAccrualPricerByBgm(Real correlation,
                                                     std::shared_ptr<SmileSection> smilesOnExpiry,
                                                     std::shared_ptr<SmileSection> smilesOnPayment,
                                                     bool withSmile,
                                                     bool byCallSpread)
    : correlation_(correlation), withSmile_(withSmile), byCallSpread_(byCallSpread),
      smilesOnExpiry_(std::move(smilesOnExpiry)), smilesOnPayment_(std::move(smilesOnPayment)) {}
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

    Real RangeAccrualPricerByBgm::derLambdaDerLambdaS(Real U) const {

        if (startTime_>0) {
            Real q = (endTime_-U)/accrualFactor_;
            return q;
        } else
            return 0.0;

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

    Real RangeAccrualPricerByBgm::derLambdaDerLambdaT(Real U) const {

        if (startTime_>0) {
            Real p = (U-startTime_)/accrualFactor_;
            return p;
        } else
            return 0.0;

    }

    Real RangeAccrualPricerByBgm::digitalRangePrice(Real lowerTrigger,
                                                      Real upperTrigger,
                                                      Real initialValue,
                                                      Real expiry,
                                                      Real deflator) const{
            const Real lowerPrice = digitalPrice(lowerTrigger, initialValue, expiry, deflator);
            const Real upperPrice = digitalPrice(upperTrigger, initialValue, expiry, deflator);
            const Real result =  lowerPrice - upperPrice;
            QL_REQUIRE(result >=0.,
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
        if (byCallSpread_) {

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

        QL_REQUIRE(result > -std::pow(eps_,.5),
            "RangeAccrualPricerByBgm::digitalPriceWithSmile: result< 0 Result:"<<result);
        QL_REQUIRE(result/deflator <=  1.0 + std::pow(eps_,.2),
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

        Real derLambdaDerK = derLambdaDerLambdaS(expiry) * derSmileS +
                             derLambdaDerLambdaT(expiry) * derSmileT;
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

        QL_REQUIRE(std::fabs(result/deflator) <= 1.0 + std::pow(eps_,.2),
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


    RangeAccrualLeg::RangeAccrualLeg(Schedule schedule, std::shared_ptr<IborIndex> index)
    : schedule_(std::move(schedule)), index_(std::move(index)) {}

    RangeAccrualLeg& RangeAccrualLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withLowerTriggers(Rate trigger) {
        lowerTriggers_ = std::vector<Rate>(1,trigger);
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withLowerTriggers(
                                          const std::vector<Rate>& triggers) {
        lowerTriggers_ = triggers;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withUpperTriggers(Rate trigger) {
        upperTriggers_ = std::vector<Rate>(1,trigger);
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withUpperTriggers(
                                          const std::vector<Rate>& triggers) {
        upperTriggers_ = triggers;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withObservationTenor(
                                                        const Period& tenor) {
        observationTenor_ = tenor;
        return *this;
    }

    RangeAccrualLeg& RangeAccrualLeg::withObservationConvention(
                                           BusinessDayConvention convention) {
        observationConvention_ = convention;
        return *this;
    }

    RangeAccrualLeg::operator Leg() const {

        QL_REQUIRE(!notionals_.empty(), "no notional given");

        Size n = schedule_.size()-1;
        QL_REQUIRE(notionals_.size() <= n,
                   "too many nominals (" << notionals_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(fixingDays_.size() <= n,
                   "too many fixingDays (" << fixingDays_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(gearings_.size()<=n,
                   "too many gearings (" << gearings_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(spreads_.size()<=n,
                   "too many spreads (" << spreads_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(lowerTriggers_.size()<=n,
                   "too many lowerTriggers (" << lowerTriggers_.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(upperTriggers_.size()<=n,
                   "too many upperTriggers (" << upperTriggers_.size() <<
                   "), only " << n << " required");

        Leg leg(n);

        // the following is not always correct
        Calendar calendar = schedule_.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;
        std::vector<std::shared_ptr<Schedule> > observationsSchedules;

        for (Size i=0; i<n; ++i) {
            refStart = start = schedule_.date(i);
            refEnd   =   end = schedule_.date(i+1);
            paymentDate = calendar.adjust(end, paymentAdjustment_);
            if (i==0 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1)) {
                BusinessDayConvention bdc = schedule_.businessDayConvention();
                refStart = calendar.adjust(end - schedule_.tenor(), bdc);
            }
            if (i==n-1 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1)) {
                BusinessDayConvention bdc = schedule_.businessDayConvention();
                refEnd = calendar.adjust(start + schedule_.tenor(), bdc);
            }
            if (detail::get(gearings_, i, 1.0) == 0.0) { // fixed coupon
                leg.push_back(std::shared_ptr<CashFlow>(new
                    FixedRateCoupon(paymentDate,
                                    detail::get(notionals_, i, Null<Real>()),
                                    detail::get(spreads_, i, 0.0),
                                    paymentDayCounter_,
                                    start, end, refStart, refEnd)));
            } else { // floating coupon
                observationsSchedules.push_back(
                    std::make_shared<Schedule>(start, end,
                                 observationTenor_, calendar,
                                 observationConvention_,
                                 observationConvention_,
                                 DateGeneration::Forward, false));

                    leg.push_back(std::shared_ptr<CashFlow>(new
                       RangeAccrualFloatersCoupon(
                            paymentDate,
                            detail::get(notionals_, i, Null<Real>()),
                            index_,
                            start, end,
                            detail::get(fixingDays_, i, 2),
                            paymentDayCounter_,
                            detail::get(gearings_, i, 1.0),
                            detail::get(spreads_, i, 0.0),
                            refStart, refEnd,
                            observationsSchedules.back(),
                            detail::get(lowerTriggers_, i, Null<Rate>()),
                            detail::get(upperTriggers_, i, Null<Rate>()))));
            }
        }
        return leg;
    }

}
