/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/exercise.hpp>
#include <ql/pricingengines/barrier/analytictwoassetbarrierengine.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <utility>

namespace QuantLib {

    AnalyticTwoAssetBarrierEngine::AnalyticTwoAssetBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Handle<Quote> rho)
    : process1_(std::move(process1)), process2_(std::move(process2)), rho_(std::move(rho)) {
        registerWith(process1_);
        registerWith(process2_);
        registerWith(rho_);
    }

    void AnalyticTwoAssetBarrierEngine::calculate() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        QL_REQUIRE(payoff->strike()>0.0,"strike must be positive");

        Real spot2 = process2_->x0();
        // option is triggered by S2
        QL_REQUIRE(spot2 > 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(spot2), "barrier touched");

        Barrier::Type barrierType = arguments_.barrierType;

        switch (payoff->optionType()) {
          case Option::Call:
            switch (barrierType) {
              case Barrier::DownOut:
                results_.value = A(1,-1) +B(1,-1) ;
                break;
              case Barrier::UpOut:
                results_.value = A(1,1) + B(1,1) ;
                break;
              case Barrier::DownIn:
                results_.value = call()-(A(1,-1) +B(1,-1) );
                break;
              case Barrier::UpIn:
                results_.value = call()-(A(1,1) +B(1,1));
                break;
            }
            break;
          case Option::Put:
            switch (barrierType) {
              case Barrier::DownOut:
                results_.value = A(-1,-1)+B(-1,-1) ;
                break;
              case Barrier::UpOut:
                results_.value = A(-1,1)+B(-1,1) ;
                break;
              case Barrier::DownIn:
                results_.value = put()-(A(-1,-1) +B(-1,-1) );
                break;
              case Barrier::UpIn:
                results_.value = put()-(A(-1,1) +B(-1,1) );
                break;
            }
            break;
          default:
            QL_FAIL("unknown type");
        }
    }

    Real AnalyticTwoAssetBarrierEngine::underlying1() const {
        return process1_->x0();
    }

    Real AnalyticTwoAssetBarrierEngine::underlying2() const {
        return process2_->x0();
    }

    Real AnalyticTwoAssetBarrierEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticTwoAssetBarrierEngine::residualTime() const {
        return process1_->time(arguments_.exercise->lastDate());
    }

    Volatility AnalyticTwoAssetBarrierEngine::volatility1() const {
        return process1_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Volatility AnalyticTwoAssetBarrierEngine::volatility2() const {
        return process2_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Real AnalyticTwoAssetBarrierEngine::barrier() const {
        return arguments_.barrier;
    }

    Real AnalyticTwoAssetBarrierEngine::rho() const {
        return rho_->value();
    }

    Rate AnalyticTwoAssetBarrierEngine::riskFreeRate() const {
        return process1_->riskFreeRate()->zeroRate(residualTime(),
                                                   Continuous, NoFrequency);
    }


    Rate AnalyticTwoAssetBarrierEngine::dividendYield1() const {
        return process1_->dividendYield()->zeroRate(residualTime(),
                                                    Continuous, NoFrequency);
    }

    Rate AnalyticTwoAssetBarrierEngine::dividendYield2() const {
        return process2_->dividendYield()->zeroRate(residualTime(),
                                                    Continuous, NoFrequency);
    }

    Rate AnalyticTwoAssetBarrierEngine::costOfCarry1() const {
        return riskFreeRate() - dividendYield1();
    }

    Rate AnalyticTwoAssetBarrierEngine::costOfCarry2() const {
        return riskFreeRate() - dividendYield2();
    }

    Real AnalyticTwoAssetBarrierEngine::d1() const {
        return (std::log(underlying1()/strike())+(mu(costOfCarry1(),volatility1())+volatility1()*volatility1())*residualTime())/
            (volatility1()*std::sqrt(residualTime()));
    }

    Real AnalyticTwoAssetBarrierEngine::d2() const {
        return d1() - volatility1()*std::sqrt(residualTime());
    }

    Real AnalyticTwoAssetBarrierEngine::d3() const {
        return d1()+ (2*rho()*std::log(barrier()/underlying2()))/(volatility2()*std::sqrt(residualTime()));
    }

    Real AnalyticTwoAssetBarrierEngine::d4() const {
        return d2()+ (2*rho()*std::log(barrier()/underlying2()))/(volatility2()*std::sqrt(residualTime()));
    }

    Real AnalyticTwoAssetBarrierEngine::e1() const {
        return (std::log(barrier()/underlying2())-(mu(costOfCarry2(),volatility2())+rho()*volatility1()*volatility2())*residualTime())/
        (volatility2()*std::sqrt(residualTime()));
    }

    Real AnalyticTwoAssetBarrierEngine::e2() const {
         return e1()+rho()*volatility1()*std::sqrt(residualTime());
    }

    Real AnalyticTwoAssetBarrierEngine::e3() const {
            return e1()-(2*std::log(barrier()/underlying2()))/(volatility2()*std::sqrt(residualTime()));
    }

    Real AnalyticTwoAssetBarrierEngine::e4() const {
        return e2()-(2*std::log(barrier()/underlying2()))/(volatility2()*std::sqrt(residualTime()));
    }

    Real AnalyticTwoAssetBarrierEngine::mu(Real b, Real vol) const {
        return b-(vol*vol)/2;
    }

    Real AnalyticTwoAssetBarrierEngine::call() const {
        CumulativeNormalDistribution nd;
        return underlying1()*nd(d1())-strike()*std::exp(-riskFreeRate()*residualTime())*nd(d2());
    }

    Real AnalyticTwoAssetBarrierEngine::put() const {
        CumulativeNormalDistribution nd;
        return strike()*std::exp(-riskFreeRate()*residualTime())*nd(-d2())-underlying1()*nd(-d1());
    }

    Real AnalyticTwoAssetBarrierEngine::A(Real eta, Real phi) const {
        Real S1 = underlying1(), S2 = underlying2();
        Rate b1 = costOfCarry1(), b2 = costOfCarry2();
        Rate r = riskFreeRate();
        Time T = residualTime();
        Real H = barrier(), X = strike();
        Volatility sigma1 = volatility1(), sigma2 = volatility2();
        Real rho = rho_->value();

        Rate mu1 = b1 - sigma1*sigma1/2.0;
        Rate mu2 = b2 - sigma2*sigma2/2.0;

        Real d1 = (std::log(S1/X)+(mu1+sigma1*sigma1)*T)/
            (sigma1*std::sqrt(T));
        Real d2 = d1 - sigma1*std::sqrt(T);
        Real d3 = d1 + (2*rho*std::log(H/S2))/(sigma2*std::sqrt(T));
        Real d4 = d2 + (2*rho*std::log(H/S2))/(sigma2*std::sqrt(T));

        Real e1 = (std::log(H/S2)-(mu2+rho*sigma1*sigma2)*T)/
            (sigma2*std::sqrt(T));
        Real e2 = e1 + rho*sigma1*std::sqrt(T);
        Real e3 = e1 - (2*std::log(H/S2))/(sigma2*std::sqrt(T));
        Real e4 = e2 - (2*std::log(H/S2))/(sigma2*std::sqrt(T));

        Real w =
            eta*S1*std::exp((b1-r)*T) *
            (M(eta*d1, phi*e1,-eta*phi*rho)
             -std::exp((2*(mu2+rho*sigma1*sigma2)*std::log(H/S2))/(sigma2*sigma2))
             *M(eta*d3, phi*e3, -eta*phi*rho))

            - eta*X*std::exp(-r*T) *
            (M(eta*d2, phi*e2, -eta*phi*rho)
             -std::exp((2*mu2*std::log(H/S2))/(sigma2*sigma2))*
             M(eta*d4, phi*e4, -eta*phi*rho) ) ;

        return w;
    }

    Real AnalyticTwoAssetBarrierEngine::B(Real, Real) const {
        return 0.0;
    }

    Real AnalyticTwoAssetBarrierEngine::M(Real m_a, Real m_b, Real rho) const {
        BivariateCumulativeNormalDistributionDr78 f(rho);
        return f(m_a, m_b);
    }

}

