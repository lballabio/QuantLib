/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 */

/*! \file conundrumpricer.hpp
    \brief
*/

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/functional.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

//===========================================================================//
//                          BlackVanillaOptionPricer                         //
//===========================================================================//

    BlackVanillaOptionPricer::BlackVanillaOptionPricer(
            Rate forwardValue,
            Date expiryDate,
            const Period& swapTenor,
            const ext::shared_ptr<SwaptionVolatilityStructure>& volatilityStructure) :
    forwardValue_(forwardValue), expiryDate_(expiryDate), swapTenor_(swapTenor),
        volatilityStructure_(volatilityStructure),
        smile_(volatilityStructure_->smileSection(expiryDate_, swapTenor_)) {
        QL_REQUIRE(volatilityStructure->volatilityType() == ShiftedLognormal &&
                   close_enough(volatilityStructure->shift(expiryDate,swapTenor),0.0),
                   "BlackVanillaOptionPricer: zero-shift lognormal volatility required");
        }

    Real BlackVanillaOptionPricer::operator()(Real strike,
                                              Option::Type optionType,
                                              Real deflator) const {
        const Real variance = smile_->variance(strike);
        return deflator * blackFormula(optionType, strike,
            forwardValue_, std::sqrt(variance));
    }


//===========================================================================//
//                             HaganPricer                               //
//===========================================================================//
    HaganPricer::HaganPricer(const Handle<SwaptionVolatilityStructure>& swaptionVol,
                             GFunctionFactory::YieldCurveModel modelOfYieldCurve,
                             Handle<Quote> meanReversion)
    : CmsCouponPricer(swaptionVol), modelOfYieldCurve_(modelOfYieldCurve), cutoffForCaplet_(2),
      cutoffForFloorlet_(0), meanReversion_(std::move(meanReversion)) {
        registerWith(meanReversion_);
    }

    void HaganPricer::initialize(const FloatingRateCoupon& coupon){
        coupon_ =  dynamic_cast<const CmsCoupon*>(&coupon);
        QL_REQUIRE(coupon_, "CMS coupon needed");
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();
        Time accrualPeriod = coupon_->accrualPeriod();
        QL_REQUIRE(accrualPeriod != 0.0, "null accrual period");

        fixingDate_ = coupon_->fixingDate();
        paymentDate_ = coupon_->date();
        const ext::shared_ptr<SwapIndex>& swapIndex = coupon_->swapIndex();
        rateCurve_ = *(swapIndex->forwardingTermStructure());

        Date today = Settings::instance().evaluationDate();

        if(paymentDate_ > today)
            discount_ = rateCurve_->discount(paymentDate_);
        else discount_= 1.;

        spreadLegValue_ = spread_ * accrualPeriod * discount_;

        if (fixingDate_ > today){
            swapTenor_ = swapIndex->tenor();
            ext::shared_ptr<VanillaSwap> swap = swapIndex->underlyingSwap(fixingDate_);

            swapRateValue_ = swap->fairRate();

            static const Spread bp = 1.0e-4;
            annuity_ = std::fabs(swap->fixedLegBPS()/bp);

            Size q = swapIndex->fixedLegTenor().frequency();
            const Schedule& schedule = swap->fixedSchedule();
            const DayCounter& dc = swapIndex->dayCounter();
            //const DayCounter dc = coupon.dayCounter();
            Time startTime = dc.yearFraction(rateCurve_->referenceDate(),
                                             swap->startDate());
            Time swapFirstPaymentTime =
                dc.yearFraction(rateCurve_->referenceDate(), schedule.date(1));
            Time paymentTime = dc.yearFraction(rateCurve_->referenceDate(),
                                               paymentDate_);
            Real delta = (paymentTime-startTime) / (swapFirstPaymentTime-startTime);

            switch (modelOfYieldCurve_) {
                case GFunctionFactory::Standard:
                    gFunction_ = GFunctionFactory::newGFunctionStandard(q, delta, swapTenor_.length());
                    break;
                case GFunctionFactory::ExactYield:
                    gFunction_ = GFunctionFactory::newGFunctionExactYield(*coupon_);
                    break;
                case GFunctionFactory::ParallelShifts: {
                    Handle<Quote> nullMeanReversionQuote(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
                    gFunction_ = GFunctionFactory::newGFunctionWithShifts(*coupon_, nullMeanReversionQuote);
                    }
                    break;
                case GFunctionFactory::NonParallelShifts:
                    gFunction_ = GFunctionFactory::newGFunctionWithShifts(*coupon_, meanReversion_);
                    break;
                default:
                    QL_FAIL("unknown/illegal gFunction type");
            }
            vanillaOptionPricer_= ext::shared_ptr<VanillaOptionPricer>(new
                BlackVanillaOptionPricer(swapRateValue_, fixingDate_, swapTenor_,
                                        *swaptionVolatility()));
         }
    }

    Real HaganPricer::meanReversion() const { return meanReversion_->value();}

    Rate HaganPricer::swapletRate() const {
        return swapletPrice()/(coupon_->accrualPeriod()*discount_);
    }

    Real HaganPricer::capletPrice(Rate effectiveCap) const {
        // caplet is equivalent to call option on fixing
        Date today = Settings::instance().evaluationDate();
        if (fixingDate_ <= today) {
            // the fixing is determined
            const Rate Rs =
                std::max(coupon_->swapIndex()->fixing(fixingDate_)-effectiveCap, 0.);
            Rate price = (gearing_*Rs)*(coupon_->accrualPeriod()*discount_);
            return price;
        } else {
            Real cutoffNearZero = 1e-10;
            Real capletPrice = 0;
            if (effectiveCap < cutoffForCaplet_) {
                Rate effectiveStrikeForMax = std::max(effectiveCap,cutoffNearZero);
                capletPrice = optionletPrice(Option::Call, effectiveStrikeForMax);
            }
            return gearing_ * capletPrice;
        }
    }

    Rate HaganPricer::capletRate(Rate effectiveCap) const {
        return capletPrice(effectiveCap)/(coupon_->accrualPeriod()*discount_);
    }

    Real HaganPricer::floorletPrice(Rate effectiveFloor) const {
        // floorlet is equivalent to put option on fixing
        Date today = Settings::instance().evaluationDate();
        if (fixingDate_ <= today) {
            // the fixing is determined
            const Rate Rs =
                std::max(effectiveFloor-coupon_->swapIndex()->fixing(fixingDate_),0.);
            Rate price = (gearing_*Rs)*(coupon_->accrualPeriod()*discount_);
            return price;
        } else {
            Real cutoffNearZero = 1e-10;
            Real floorletPrice = 0;
            if (effectiveFloor > cutoffForFloorlet_){
                Rate effectiveStrikeForMin = std::max(effectiveFloor,cutoffNearZero);
                floorletPrice=optionletPrice(Option::Put, effectiveStrikeForMin);
            }
            return gearing_ * floorletPrice;
        }
    }

    Rate HaganPricer::floorletRate(Rate effectiveFloor) const {
        return floorletPrice(effectiveFloor)/(coupon_->accrualPeriod()*discount_);
    }

//===========================================================================//
//                  NumericHaganPricer                    //
//===========================================================================//

    namespace {

        class VariableChange {
          public:
            VariableChange(ext::function<Real (Real)>& f,
                           Real a, Real b, Size k)
            : a_(a), width_(b-a), f_(f), k_(k) {}
            Real value(Real x) const {
                Real newVar;
                Real temp = width_;
                for (Size i = 1; i < k_ ; ++i) {
                    temp *= x;
                }
                newVar = a_ + x* temp;
                return f_(newVar) * k_* temp;
            }
          private:
            Real a_, width_;
            ext::function<Real (Real)> f_;
            Size k_;
        };

        class Spy {
          public:
            explicit Spy(ext::function<Real(Real)> f) : f_(std::move(f)) {}
            Real value(Real x){
                abscissas.push_back(x);
                Real value = f_(x);
                functionValues.push_back(value);
                return value;
            }
          private:
            ext::function<Real (Real)> f_;
            std::vector<Real> abscissas;
            std::vector<Real> functionValues;
        };

    }

    NumericHaganPricer::NumericHaganPricer(
        const Handle<SwaptionVolatilityStructure>& swaptionVol,
        GFunctionFactory::YieldCurveModel modelOfYieldCurve,
        const Handle<Quote>& meanReversion,
        Real lowerLimit,
        Real upperLimit,
        Real precision,
        Real hardUpperLimit)
    : HaganPricer(swaptionVol, modelOfYieldCurve, meanReversion),
       upperLimit_(upperLimit),
       lowerLimit_(lowerLimit),
       requiredStdDeviations_(8),
       precision_(precision),
       refiningIntegrationTolerance_(.0001),
       hardUpperLimit_(hardUpperLimit) {

    }

    Real NumericHaganPricer::integrate(Real a, Real b, const ConundrumIntegrand& integrand) const {

            Real result =.0;
            //double abserr =.0;
            //double alpha = 1.0;


            //double epsabs = precision_;
            //double epsrel = 1.0; // we are interested only in absolute precision
            //size_t neval =0;

            // we use the non adaptive algorithm only for semi infinite interval
            if (a>0){

                // we estimate the actual boudary by testing integrand values
                Real upperBoundary = 2*a;
                while(integrand(upperBoundary)>precision_)
                    upperBoundary *=2.0;
                // sometimes b < a because of a wrong estimation of b based on stdev
                if (b > a)
                    upperBoundary = std::min(upperBoundary, b);

                ext::function<Real (Real)> f;
                GaussKronrodNonAdaptive
                    gaussKronrodNonAdaptive(precision_, 1000000, 1.0);
                // if the integration intervall is wide enough we use the
                // following change variable x -> a + (b-a)*(t/(a-b))^3
                upperBoundary = std::max(a,std::min(upperBoundary, hardUpperLimit_));
                if (upperBoundary > 2*a){
                    Size k = 3;
                    ext::function<Real (Real)> temp = ext::cref(integrand);
                    VariableChange variableChange(temp, a, upperBoundary, k);
                    f = [&](Real _x) { return variableChange.value(_x); };
                    result = gaussKronrodNonAdaptive(f, .0, 1.0);
                } else {
                    f = ext::cref(integrand);
                    result = gaussKronrodNonAdaptive(f, a, upperBoundary);
                }

                // if the expected precision has not been reached we use the old algorithm
                if (!gaussKronrodNonAdaptive.integrationSuccess()){
                    const GaussKronrodAdaptive integrator(precision_, 100000);
                    b = std::max(a,std::min(b, hardUpperLimit_));
                    result = integrator(integrand,a , b);
                }

            } else {   // if a < b we use the old algorithm
                b = std::max(a,std::min(b,hardUpperLimit_));
                const GaussKronrodAdaptive integrator(precision_, 100000);
                result = integrator(integrand,a , b);
            }
            return result;
    }

    Real NumericHaganPricer::optionletPrice(
                                Option::Type optionType, Real strike) const {

        ext::shared_ptr<ConundrumIntegrand> integrand(new
            ConundrumIntegrand(vanillaOptionPricer_, rateCurve_, gFunction_,
                               fixingDate_, paymentDate_, annuity_,
                               swapRateValue_, strike, optionType));
        stdDeviationsForUpperLimit_= requiredStdDeviations_;
        Real a, b, integralValue;
        if (optionType==Option::Call) {
            upperLimit_ = resetUpperLimit(stdDeviationsForUpperLimit_);
        //    while(upperLimit_ <= strike){
        //        stdDeviationsForUpperLimit_ += 1.;
        //        upperLimit_ = resetUpperLimit(stdDeviationsForUpperLimit_);
        //    }
            integralValue = integrate(strike, upperLimit_, *integrand);
            //refineIntegration(integralValue, *integrand);
        } else {
            a = std::min(strike, lowerLimit_);
            b = strike;
            integralValue = integrate(a, b, *integrand);
        }

        Real dFdK = integrand->firstDerivativeOfF(strike);
        Real swaptionPrice =
            (*vanillaOptionPricer_)(strike, optionType, annuity_);

        // v. HAGAN, Conundrums..., formule 2.17a, 2.18a
        return coupon_->accrualPeriod() * (discount_/annuity_) *
            ((1 + dFdK) * swaptionPrice + static_cast<Real>(optionType) * integralValue);
    }

    Real NumericHaganPricer::swapletPrice() const {

        Date today = Settings::instance().evaluationDate();
        if (fixingDate_ <= today) {
            // the fixing is determined
            const Rate Rs = coupon_->swapIndex()->fixing(fixingDate_);
            Rate price = (gearing_*Rs + spread_)*(coupon_->accrualPeriod()*discount_);
            return price;
        } else {
            Real atmCapletPrice = optionletPrice(Option::Call, swapRateValue_);
            Real atmFloorletPrice = optionletPrice(Option::Put, swapRateValue_);
            return gearing_ *(coupon_->accrualPeriod()* discount_ * swapRateValue_
                             + atmCapletPrice - atmFloorletPrice)
                   + spreadLegValue_;
        }
    }

    Real NumericHaganPricer::refineIntegration(Real integralValue,
                                                const ConundrumIntegrand& integrand) const {
        Real percDiff = 1000.;
        while(std::fabs(percDiff) < refiningIntegrationTolerance_){
            stdDeviationsForUpperLimit_ += 1.;
            Real lowerLimit = upperLimit_;
            upperLimit_ = resetUpperLimit(stdDeviationsForUpperLimit_);
            Real diff = integrate(lowerLimit, upperLimit_,integrand);
            percDiff = diff/integralValue;
            integralValue += diff;
        }
        return integralValue;
    }

    Real NumericHaganPricer::resetUpperLimit(
                        Real stdDeviationsForUpperLimit) const {
        //return 1.0;
        Real variance =
            swaptionVolatility()->blackVariance(fixingDate_,swapTenor_,swapRateValue_);
        return swapRateValue_ *
            std::exp(stdDeviationsForUpperLimit*std::sqrt(variance));
    }


//===========================================================================//
//                              ConundrumIntegrand                           //
//===========================================================================//

    NumericHaganPricer::ConundrumIntegrand::ConundrumIntegrand(
        ext::shared_ptr<VanillaOptionPricer> o,
        const ext::shared_ptr<YieldTermStructure>&,
        ext::shared_ptr<GFunction> gFunction,
        Date fixingDate,
        Date paymentDate,
        Real annuity,
        Real forwardValue,
        Real strike,
        Option::Type optionType)
    : vanillaOptionPricer_(std::move(o)), forwardValue_(forwardValue), annuity_(annuity),
      fixingDate_(fixingDate), paymentDate_(paymentDate), strike_(strike), optionType_(optionType),
      gFunction_(std::move(gFunction)) {}

    void NumericHaganPricer::ConundrumIntegrand::setStrike(Real strike) {
        strike_ = strike;
    }

    Real NumericHaganPricer::ConundrumIntegrand::strike() const {
        return strike_;
    }

    Real NumericHaganPricer::ConundrumIntegrand::annuity() const {
        return annuity_;
    }

    Date NumericHaganPricer::ConundrumIntegrand::fixingDate() const {
        return fixingDate_;
    }

    Real NumericHaganPricer::ConundrumIntegrand::functionF (const Real x) const {
        const Real Gx = (*gFunction_)(x);
        const Real GR = (*gFunction_)(forwardValue_);
        return (x - strike_) * (Gx/GR - 1.0);
    }

    Real NumericHaganPricer::ConundrumIntegrand::firstDerivativeOfF (const Real x) const {
        const Real Gx = (*gFunction_)(x);
        const Real GR = (*gFunction_)(forwardValue_) ;
        const Real G1 = gFunction_->firstDerivative(x);
        return (Gx/GR - 1.0) + G1/GR * (x - strike_);
    }

    Real NumericHaganPricer::ConundrumIntegrand::secondDerivativeOfF (const Real x) const {
        const Real GR = (*gFunction_)(forwardValue_) ;
        const Real G1 = gFunction_->firstDerivative(x);
        const Real G2 = gFunction_->secondDerivative(x);
        return 2.0 * G1/GR + (x - strike_) * G2/GR;
    }

    Real NumericHaganPricer::ConundrumIntegrand::operator()(Real x) const {
        const Real option = (*vanillaOptionPricer_)(x, optionType_, annuity_);
        return option * secondDerivativeOfF(x);
    }



//===========================================================================//
//                          AnalyticHaganPricer                           //
//===========================================================================//

    AnalyticHaganPricer::AnalyticHaganPricer(
        const Handle<SwaptionVolatilityStructure>& swaptionVol,
        GFunctionFactory::YieldCurveModel modelOfYieldCurve,
        const Handle<Quote>& meanReversion)
    : HaganPricer(swaptionVol, modelOfYieldCurve, meanReversion)
      { }

    //Hagan, 3.5b, 3.5c
    Real AnalyticHaganPricer::optionletPrice(Option::Type optionType,
                                                  Real strike) const {
        Real variance = swaptionVolatility()->blackVariance(fixingDate_,
                                                           swapTenor_,
                                                           swapRateValue_);
        Real firstDerivativeOfGAtForwardValue = gFunction_->firstDerivative(
                                                        swapRateValue_);
        Real price = 0;

        Real CK = (*vanillaOptionPricer_)(strike, optionType, annuity_);
        price += (discount_/annuity_)*CK;
        const Real sqrtSigma2T = std::sqrt(variance);
        const Real lnRoverK =  std::log(swapRateValue_/strike);
        const Real d32 = (lnRoverK+1.5*variance)/sqrtSigma2T;
        const Real d12 =  (lnRoverK+.5*variance)/sqrtSigma2T;
        const Real dminus12 =  (lnRoverK-.5*variance)/sqrtSigma2T;
        const Real optionSign = static_cast<Real>(optionType);

        CumulativeNormalDistribution cumulativeOfNormal;
        const Real N32 = cumulativeOfNormal(optionSign*d32);
        const Real N12 = cumulativeOfNormal(optionSign*d12);
        const Real Nminus12 = cumulativeOfNormal(optionSign*dminus12);

        price += optionSign * firstDerivativeOfGAtForwardValue * annuity_ *
            swapRateValue_ * (swapRateValue_ * std::exp(variance) * N32-
            (swapRateValue_+strike) * N12 + strike * Nminus12);
        price *= coupon_->accrualPeriod();
        return price;
    }

    //Hagan 3.4c
    Real AnalyticHaganPricer::swapletPrice() const {

        Date today = Settings::instance().evaluationDate();
        if (fixingDate_ <= today) {
            // the fixing is determined
            const Rate Rs = coupon_->swapIndex()->fixing(fixingDate_);
            Rate price = (gearing_*Rs + spread_)*(coupon_->accrualPeriod()*discount_);
            return price;
        } else {
            Real variance(swaptionVolatility()->blackVariance(fixingDate_,
                                                               swapTenor_,
                                                               swapRateValue_));
            Real firstDerivativeOfGAtForwardValue(gFunction_->firstDerivative(
                                                            swapRateValue_));
            Real price = 0;
            price += discount_*swapRateValue_;
            price += firstDerivativeOfGAtForwardValue*annuity_*swapRateValue_*
                     swapRateValue_*(std::exp(variance)-1.);
            return gearing_ * price * coupon_->accrualPeriod() + spreadLegValue_;
        }
    }


//===========================================================================//
//                              GFunctionStandard                            //
//===========================================================================//

    Real GFunctionFactory::GFunctionStandard::operator()(Real x) {
        Real n = static_cast<Real>(swapLength_) * q_;
        return x / std::pow((1.0 + x/q_), delta_) * 1.0 /
            (1.0 - 1.0 / std::pow((1.0 + x/q_), n));
    }

    Real GFunctionFactory::GFunctionStandard::firstDerivative(Real x) {
        Real n = static_cast<Real>(swapLength_) * q_;
        Real a = 1.0 + x / q_;
        Real AA = a - delta_/q_ * x;
        Real B = std::pow(a,(n - delta_ - 1.0))/(std::pow(a,n) - 1.0);

        Real secNum = n * x * std::pow(a,(n-1.0));
        Real secDen = q_ * std::pow(a, delta_) * (std::pow(a, n) - 1.0) *
            (std::pow(a, n) - 1.0);
        Real sec = secNum / secDen;

        return AA * B - sec;
    }

    Real GFunctionFactory::GFunctionStandard::secondDerivative(Real x) {
        Real n = static_cast<Real>(swapLength_) * q_;
        Real a = 1.0 + x/q_;
        Real AA = a - delta_/q_ * x;
        Real A1 = (1.0 - delta_)/q_;
        Real B = std::pow(a,(n - delta_ - 1.0))/(std::pow(a,n) - 1.0);
        Real Num = (1.0 + delta_ - n) * std::pow(a, (n-delta_-2.0)) -
            (1.0 + delta_) * std::pow(a, (2.0*n-delta_-2.0));
        Real Den = (std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0);
        Real B1 = 1.0 / q_ * Num / Den;

        Real C =  x / std::pow(a, delta_);
        Real C1 = (std::pow(a, delta_)
            - delta_ /q_ * x * std::pow(a, (delta_ - 1.0))) / std::pow(a, 2 * delta_);

        Real D =  std::pow(a, (n-1.0))/ ((std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0));
        Real D1 = ((n - 1.0) * std::pow(a, (n-2.0)) * (std::pow(a, n) - 1.0)
            - 2 * n * std::pow(a, (2 * (n-1.0))))
            / (q_ * (std::pow(a, n) - 1.0)*(std::pow(a, n) - 1.0)*(std::pow(a, n) - 1.0));

        return A1 * B + AA * B1 - n/q_ * (C1 * D + C * D1);
    }

    ext::shared_ptr<GFunction> GFunctionFactory::newGFunctionStandard(Size q,
                                                            Real delta, Size swapLength) {
        return ext::shared_ptr<GFunction>(new GFunctionStandard(q, delta, swapLength));
    }

//===========================================================================//
//                              GFunctionExactYield                          //
//===========================================================================//

    GFunctionFactory::GFunctionExactYield::GFunctionExactYield(const CmsCoupon& coupon){

        const ext::shared_ptr<SwapIndex>& swapIndex = coupon.swapIndex();
        const ext::shared_ptr<VanillaSwap>& swap =
            swapIndex->underlyingSwap(coupon.fixingDate());

        const Schedule& schedule = swap->fixedSchedule();
        Handle<YieldTermStructure> rateCurve =
            swapIndex->forwardingTermStructure();

        const DayCounter& dc = swapIndex->dayCounter();

        Real swapStartTime = dc.yearFraction(rateCurve->referenceDate(),
                                             schedule.startDate());
        Real swapFirstPaymentTime = dc.yearFraction(rateCurve->referenceDate(),
                                                    schedule.date(1));

        Real paymentTime = dc.yearFraction(rateCurve->referenceDate(),
                                                 coupon.date());

        delta_ = (paymentTime-swapStartTime) / (swapFirstPaymentTime-swapStartTime);

        const Leg& fixedLeg(swap->fixedLeg());
        Size n = fixedLeg.size();
        accruals_.reserve(n);
        for (Size i=0; i<n; ++i) {
            ext::shared_ptr<Coupon> cpn =
                ext::dynamic_pointer_cast<Coupon>(fixedLeg[i]);
            accruals_.push_back(cpn->accrualPeriod());
        }
    }

    Real GFunctionFactory::GFunctionExactYield::operator()(Real x) {
        Real product = 1.;
        for (double accrual : accruals_) {
            product *= 1. / (1. + accrual * x);
        }
        return x*std::pow(1.+ accruals_[0]*x,-delta_)*(1./(1.-product));
    }

    Real GFunctionFactory::GFunctionExactYield::firstDerivative(Real x) {
        Real c = -1.;
        Real derC = 0.;
        std::vector<Real> b;
        b.reserve(accruals_.size());
        for (double accrual : accruals_) {
            Real temp = 1.0 / (1.0 + accrual * x);
            b.push_back(temp);
            c *= temp;
            derC += accrual * temp;
        }
        c += 1.;
        c = 1./c;
        derC *= (c-c*c);

        return -delta_*accruals_[0]*std::pow(b[0],delta_+1.)*x*c+
                std::pow(b[0],delta_)*c+ std::pow(b[0],delta_)*x*derC;
        //Real dx = 1.0e-8;
        //return (operator()(x+dx)-operator()(x-dx))/(2.0*dx);
    }

    Real GFunctionFactory::GFunctionExactYield::secondDerivative(Real x) {
        Real c = -1.;
        Real sum = 0.;
        Real sumOfSquare = 0.;
        std::vector<Real> b;
        b.reserve(accruals_.size());
        for (double accrual : accruals_) {
            Real temp = 1.0 / (1.0 + accrual * x);
            b.push_back(temp);
            c *= temp;
            sum += accrual * temp;
            sumOfSquare += std::pow(accrual * temp, 2.0);
        }
        c += 1.;
        c = 1./c;
        Real derC =sum*(c-c*c);

        return (-delta_*accruals_[0]*std::pow(b[0],delta_+1.)*c+ std::pow(b[0],delta_)*derC)*
               (-delta_*accruals_[0]*b[0]*x + 1. + x*(1.-c)*sum)+
                std::pow(b[0],delta_)*c*(delta_*std::pow(accruals_[0]*b[0],2.)*x - delta_* accruals_[0]*b[0] -
                x*derC*sum + (1.-c)*sum - x*(1.-c)*sumOfSquare);
        //Real dx = 1.0e-8;
        //return (firstDerivative(x+dx)-firstDerivative(x-dx))/(2.0*dx);
    }

    ext::shared_ptr<GFunction> GFunctionFactory::newGFunctionExactYield(const CmsCoupon& coupon) {
        return ext::shared_ptr<GFunction>(new GFunctionExactYield(coupon));
    }



//===========================================================================//
//                            GFunctionWithShifts                            //
//===========================================================================//

    GFunctionFactory::GFunctionWithShifts::GFunctionWithShifts(const CmsCoupon& coupon,
                                                               Handle<Quote> meanReversion)
    : meanReversion_(std::move(meanReversion)), calibratedShift_(0.03), tmpRs_(10000000.0),
      accuracy_(1.0e-14) {

        const ext::shared_ptr<SwapIndex>& swapIndex = coupon.swapIndex();
        const ext::shared_ptr<VanillaSwap>& swap = swapIndex->underlyingSwap(coupon.fixingDate());

        swapRateValue_ = swap->fairRate();

        objectiveFunction_ = ext::make_shared<ObjectiveFunction>(*this, swapRateValue_);

        const Schedule& schedule = swap->fixedSchedule();
        Handle<YieldTermStructure> rateCurve =
            swapIndex->forwardingTermStructure();
        const DayCounter& dc = swapIndex->dayCounter();

        swapStartTime_ = dc.yearFraction(rateCurve->referenceDate(),
                                         schedule.startDate());
        discountAtStart_ = rateCurve->discount(schedule.startDate());

        Real paymentTime = dc.yearFraction(rateCurve->referenceDate(),
                                                 coupon.date());

        shapedPaymentTime_ = shapeOfShift(paymentTime);

        const Leg& fixedLeg(swap->fixedLeg());
        Size n = fixedLeg.size();
        accruals_.reserve(n);
        shapedSwapPaymentTimes_.reserve(n);
        swapPaymentDiscounts_.reserve(n);
        for(Size i=0; i<n; ++i) {
            ext::shared_ptr<Coupon> cpn =
                ext::dynamic_pointer_cast<Coupon>(fixedLeg[i]);
            accruals_.push_back(cpn->accrualPeriod());
            const Date paymentDate(cpn->date());
            const double swapPaymentTime(dc.yearFraction(rateCurve->referenceDate(), paymentDate));
            shapedSwapPaymentTimes_.push_back(shapeOfShift(swapPaymentTime));
            swapPaymentDiscounts_.push_back(rateCurve->discount(paymentDate));
        }
        discountRatio_ = swapPaymentDiscounts_.back()/discountAtStart_;
    }

    Real GFunctionFactory::GFunctionWithShifts::operator()(Real Rs) {
        const Real calibratedShift = calibrationOfShift(Rs);
        return Rs* functionZ(calibratedShift);
    }

    Real GFunctionFactory::GFunctionWithShifts::functionZ(Real x) {
        return std::exp(-shapedPaymentTime_*x)
            / (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x));
    }

    Real GFunctionFactory::GFunctionWithShifts::derRs_derX(Real x) {
        Real sqrtDenominator = 0;
        Real derSqrtDenominator = 0;
        for(Size i=0; i<accruals_.size(); i++) {
            sqrtDenominator += accruals_[i]*swapPaymentDiscounts_[i]
                *std::exp(-shapedSwapPaymentTimes_[i]*x);
            derSqrtDenominator -= shapedSwapPaymentTimes_[i]* accruals_[i]*swapPaymentDiscounts_[i]
                *std::exp(-shapedSwapPaymentTimes_[i]*x);
        }
        const Real denominator = sqrtDenominator* sqrtDenominator;

        Real numerator = 0;
        numerator += shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()*
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*sqrtDenominator;
        numerator -= (discountAtStart_ - swapPaymentDiscounts_.back()* std::exp(-shapedSwapPaymentTimes_.back()*x))*
                     derSqrtDenominator;
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::derRs_derX: denominator == 0");
        return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::der2Rs_derX2(Real x) {
        Real denOfRfunztion = 0.;
        Real derDenOfRfunztion = 0.;
        Real der2DenOfRfunztion = 0.;
        for(Size i=0; i<accruals_.size(); i++) {
            denOfRfunztion += accruals_[i]*swapPaymentDiscounts_[i]
                *std::exp(-shapedSwapPaymentTimes_[i]*x);
            derDenOfRfunztion -= shapedSwapPaymentTimes_[i]* accruals_[i]*swapPaymentDiscounts_[i]
                *std::exp(-shapedSwapPaymentTimes_[i]*x);
            der2DenOfRfunztion+= shapedSwapPaymentTimes_[i]*shapedSwapPaymentTimes_[i]* accruals_[i]*
                swapPaymentDiscounts_[i]*std::exp(-shapedSwapPaymentTimes_[i]*x);
        }

        const Real denominator = std::pow(denOfRfunztion, 4);

        Real numOfDerR = 0;
        numOfDerR += shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()*
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*denOfRfunztion;
        numOfDerR -= (discountAtStart_ - swapPaymentDiscounts_.back()* std::exp(-shapedSwapPaymentTimes_.back()*x))*
                     derDenOfRfunztion;

        const Real denOfDerR = std::pow(denOfRfunztion,2);

        Real derNumOfDerR = 0.;
        derNumOfDerR -= shapedSwapPaymentTimes_.back()*shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()*
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*denOfRfunztion;
        derNumOfDerR += shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()*
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*derDenOfRfunztion;

        derNumOfDerR -= (shapedSwapPaymentTimes_.back()*swapPaymentDiscounts_.back()*
                        std::exp(-shapedSwapPaymentTimes_.back()*x))* derDenOfRfunztion;
        derNumOfDerR -= (discountAtStart_ - swapPaymentDiscounts_.back()* std::exp(-shapedSwapPaymentTimes_.back()*x))*
                     der2DenOfRfunztion;

        const Real derDenOfDerR = 2*denOfRfunztion*derDenOfRfunztion;

        const Real numerator = derNumOfDerR*denOfDerR -numOfDerR*derDenOfDerR;
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::der2Rs_derX2: denominator == 0");
        return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::derZ_derX(Real x) {
        const Real sqrtDenominator = (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x));
        const Real denominator = sqrtDenominator* sqrtDenominator;
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::derZ_derX: denominator == 0");

        Real numerator = 0;
        numerator -= shapedPaymentTime_* std::exp(-shapedPaymentTime_*x)* sqrtDenominator;
        numerator -= shapedSwapPaymentTimes_.back()* std::exp(-shapedPaymentTime_*x)* (1.-sqrtDenominator);

        return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::der2Z_derX2(Real x) {
        const Real denOfZfunction = (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x));
        const Real derDenOfZfunction = shapedSwapPaymentTimes_.back()*discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x);
        const Real denominator = std::pow(denOfZfunction, 4);
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::der2Z_derX2: denominator == 0");

        Real numOfDerZ = 0;
        numOfDerZ -= shapedPaymentTime_* std::exp(-shapedPaymentTime_*x)* denOfZfunction;
        numOfDerZ -= shapedSwapPaymentTimes_.back()* std::exp(-shapedPaymentTime_*x)* (1.-denOfZfunction);

        const Real denOfDerZ = std::pow(denOfZfunction,2);
        const Real derNumOfDerZ = (-shapedPaymentTime_* std::exp(-shapedPaymentTime_*x)*
                             (-shapedPaymentTime_+(shapedPaymentTime_*discountRatio_-
                               shapedSwapPaymentTimes_.back()*discountRatio_)* std::exp(-shapedSwapPaymentTimes_.back()*x))
                              -shapedSwapPaymentTimes_.back()*std::exp(-shapedPaymentTime_*x)*
                              (shapedPaymentTime_*discountRatio_- shapedSwapPaymentTimes_.back()*discountRatio_)*
                              std::exp(-shapedSwapPaymentTimes_.back()*x));

        const Real derDenOfDerZ = 2*denOfZfunction*derDenOfZfunction;
        const Real numerator = derNumOfDerZ*denOfDerZ -numOfDerZ*derDenOfDerZ;

        return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::firstDerivative(Real Rs) {
        //Real dRs = 1.0e-8;
        //return (operator()(Rs+dRs)-operator()(Rs-dRs))/(2.0*dRs);
        const Real calibratedShift = calibrationOfShift(Rs);
        return functionZ(calibratedShift) + Rs * derZ_derX(calibratedShift)/derRs_derX(calibratedShift);
    }

    Real GFunctionFactory::GFunctionWithShifts::secondDerivative(Real Rs) {
        //Real dRs = 1.0e-8;
        //return (firstDerivative(Rs+dRs)-firstDerivative(Rs-dRs))/(2.0*dRs);
        const Real calibratedShift = calibrationOfShift(Rs);
        return 2.*derZ_derX(calibratedShift)/derRs_derX(calibratedShift) +
            Rs * der2Z_derX2(calibratedShift)/std::pow(derRs_derX(calibratedShift),2.)-
            Rs * derZ_derX(calibratedShift)*der2Rs_derX2(calibratedShift)/
            std::pow(derRs_derX(calibratedShift),3.);
    }

    Real GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::operator ()(const Real& x) const {
        Real result = 0;
        derivative_ = 0;
        for(Size i=0; i<o_.accruals_.size(); i++) {
            Real temp = o_.accruals_[i]*o_.swapPaymentDiscounts_[i]
                *std::exp(-o_.shapedSwapPaymentTimes_[i]*x);
            result += temp;
            derivative_ -= o_.shapedSwapPaymentTimes_[i] * temp;
        }
        result *= Rs_;
        derivative_ *= Rs_;
        Real temp = o_.swapPaymentDiscounts_.back()
            * std::exp(-o_.shapedSwapPaymentTimes_.back()*x);

        result += temp-o_.discountAtStart_;
        derivative_ -= o_.shapedSwapPaymentTimes_.back()*temp;
        return result;
    }

    Real GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::derivative(const Real&) const {
        return derivative_;
    }

    void GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::setSwapRateValue(Real x) {
        Rs_ = x;
    }

    Real GFunctionFactory::GFunctionWithShifts::shapeOfShift(Real s) const {
        const Real x(s-swapStartTime_);
        Real meanReversion = meanReversion_->value();
        if(meanReversion>0) {
            return (1.-std::exp(-meanReversion*x))/meanReversion;
        }
        else {
            return x;
        }
    }

    Real GFunctionFactory::GFunctionWithShifts::calibrationOfShift(Real Rs){

        if(Rs!=tmpRs_){
            Real initialGuess, N=0, D=0;
            for(Size i=0; i<accruals_.size(); i++) {
                N+=accruals_[i]*swapPaymentDiscounts_[i];
                D+=accruals_[i]*swapPaymentDiscounts_[i]*shapedSwapPaymentTimes_[i];
            }
            N *= Rs;
            D *= Rs;
            N += accruals_.back() * swapPaymentDiscounts_.back()
                - objectiveFunction_->gFunctionWithShifts().discountAtStart_;
            D += accruals_.back() * swapPaymentDiscounts_.back()*
                            shapedSwapPaymentTimes_.back();
            initialGuess = N/D;

            objectiveFunction_->setSwapRateValue(Rs);
            Newton solver;
            solver.setMaxEvaluations(1000);

            // these boundaries migth not be big enough if the volatility
            // of big swap rate values is too high . In this case the G function
            // is not even integrable, so better to fix the vol than increasing
            // these values
            const Real lower = -20, upper = 20.;

            try {
                calibratedShift_ = solver.solve(*objectiveFunction_, accuracy_,
                    std::max( std::min(initialGuess, upper*.99), lower*.99),
                    lower, upper);
            } catch (std::exception& e) {
                QL_FAIL("meanReversion: " << meanReversion_->value() <<
                        ", swapRateValue: " << swapRateValue_ <<
                        ", swapStartTime: " << swapStartTime_ <<
                        ", shapedPaymentTime: " << shapedPaymentTime_ <<
                        "\n error message: " << e.what());
            }
            tmpRs_=Rs;
        }
        return calibratedShift_;
    }

    ext::shared_ptr<GFunction> GFunctionFactory::newGFunctionWithShifts(const CmsCoupon& coupon,
                                                                          const Handle<Quote>& meanReversion) {
        return ext::shared_ptr<GFunction>(new GFunctionWithShifts(coupon, meanReversion));
    }

}
