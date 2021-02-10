/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian

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
#include <ql/experimental/barrieroption/wulinyongdoublebarrierengine.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <utility>

namespace QuantLib {

    WulinYongDoubleBarrierEngine::WulinYongDoubleBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process, int series)
    : process_(std::move(process)), series_(series) {
        registerWith(process_);
    }

    void WulinYongDoubleBarrierEngine::calculate() const {

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        QL_REQUIRE(payoff->strike()>0.0,
                   "strike must be positive");

        Real K = payoff->strike();
        Real S = process_->x0();
        QL_REQUIRE(S >= 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(S), "barrier touched");

        DoubleBarrier::Type barrierType = arguments_.barrierType;
        QL_REQUIRE(barrierType == DoubleBarrier::KnockOut || 
                   barrierType == DoubleBarrier::KnockIn,
                   "only KnockIn and KnockOut options supported");

        Real L = arguments_.barrier_lo;
        Real H = arguments_.barrier_hi;
        Real K_up = std::min(H, K);
        Real K_down = std::max(L, K);
        Time T = residualTime();
        Real rd = riskFreeRate();
        Real dd = riskFreeDiscount();
        Real rf = dividendYield();
        Real df = dividendDiscount();
        Real vol = volatility();
        Real mu = rd - rf - vol*vol/2.0;
        Real sgn = mu > 0 ? 1.0 :(mu < 0 ? -1.0: 0.0);
        //rebate
        Real R_L = arguments_.rebate;
        Real R_H = arguments_.rebate;

        //european option
        EuropeanOption europeanOption(payoff, arguments_.exercise);
        ext::shared_ptr<PricingEngine> analyticEuropeanEngine =
            ext::make_shared<AnalyticEuropeanEngine>(process_);
        europeanOption.setPricingEngine(analyticEuropeanEngine);
        Real european = europeanOption.NPV();

        Real barrierOut = 0;
        Real rebateIn = 0;
        for(int n = -series_; n < series_; n++){
            Real d1 = D(S/H*std::pow(L/H, 2.0*n), vol*vol+mu, vol, T);
            Real d2 = d1 - vol*std::sqrt(T);
            Real g1 = D(H/S*std::pow(L/H, 2.0*n - 1.0), vol*vol+mu, vol, T);
            Real g2 = g1 - vol*std::sqrt(T);
            Real h1 = D(S/H*std::pow(L/H, 2.0*n - 1.0), vol*vol+mu, vol, T);
            Real h2 = h1 - vol*std::sqrt(T);
            Real k1 = D(L/S*std::pow(L/H, 2.0*n - 1.0), vol*vol+mu, vol, T);
            Real k2 = k1 - vol*std::sqrt(T);
            Real d1_down = D(S/K_down*std::pow(L/H, 2.0*n), vol*vol+mu, vol, T);
            Real d2_down = d1_down - vol*std::sqrt(T);
            Real d1_up = D(S/K_up*std::pow(L/H, 2.0*n), vol*vol+mu, vol, T);
            Real d2_up = d1_up - vol*std::sqrt(T);
            Real k1_down = D((H*H)/(K_down*S)*std::pow(L/H, 2.0*n), vol*vol+mu, vol, T);
            Real k2_down = k1_down - vol*std::sqrt(T);
            Real k1_up = D((H*H)/(K_up*S)*std::pow(L/H, 2.0*n), vol*vol+mu, vol, T);
            Real k2_up = k1_up - vol*std::sqrt(T);

            if( payoff->optionType() == Option::Call) {
                barrierOut += std::pow(L/H, 2.0 * n * mu/(vol*vol))*
                            (df*S*std::pow(L/H, 2.0*n)*(f_(d1_down)-f_(d1))
                            -dd*K*(f_(d2_down)-f_(d2))
                            -df*std::pow(L/H, 2.0*n)*H*H/S*std::pow(H/S, 2.0*mu/(vol*vol))*(f_(k1_down)-f_(k1))
                            +dd*K*std::pow(H/S,2.0*mu/(vol*vol))*(f_(k2_down)-f_(k2)));
            }
            else if(payoff->optionType() == Option::Put){
                barrierOut += std::pow(L/H, 2.0 * n * mu/(vol*vol))*
                            (dd*K*(f_(h2)-f_(d2_up))
                            -df*S*std::pow(L/H, 2.0*n)*(f_(h1)-f_(d1_up))
                            -dd*K*std::pow(H/S,2.0*mu/(vol*vol))*(f_(g2)-f_(k2_up))
                            +df*std::pow(L/H, 2.0*n)*H*H/S*std::pow(H/S, 2.0*mu/(vol*vol))*(f_(g1)-f_(k1_up)));
            }
            else {
                QL_FAIL("option type not recognized");
            }

            Real v1 = D(H/S*std::pow(H/L, 2.0*n), -mu, vol, T);
            Real v2 = D(H/S*std::pow(H/L, 2.0*n), mu, vol, T);
            Real v3 = D(S/L*std::pow(H/L, 2.0*n), -mu, vol, T);
            Real v4 = D(S/L*std::pow(H/L, 2.0*n), mu, vol, T);
            rebateIn +=  dd * R_H * sgn * (std::pow(L/H, 2.0*n*mu/(vol*vol)) * f_(sgn * v1) - std::pow(H/S, 2.0*mu/(vol*vol)) * f_(-sgn * v2))
                       + dd * R_L * sgn * (std::pow(L/S, 2.0*mu/(vol*vol)) * f_(-sgn * v3) - std::pow(H/L, 2.0*n*mu/(vol*vol)) * f_(sgn * v4));
        }

        //rebate paid at maturity
        if(barrierType == DoubleBarrier::KnockOut)
            results_.value = barrierOut ;
        else
            results_.value = european - barrierOut;
        results_.additionalResults["vanilla"] = european;
        results_.additionalResults["barrierOut"] = barrierOut;
        results_.additionalResults["barrierIn"] = european - barrierOut;
        results_.additionalResults["rebateIn"] = rebateIn;
    }


    Real WulinYongDoubleBarrierEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        return payoff->strike();
    }

    Time WulinYongDoubleBarrierEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Volatility WulinYongDoubleBarrierEngine::volatility() const {
        return process_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Rate WulinYongDoubleBarrierEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
    }

    DiscountFactor WulinYongDoubleBarrierEngine::riskFreeDiscount() const {
        return process_->riskFreeRate()->discount(residualTime());
    }

    Rate WulinYongDoubleBarrierEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(residualTime(),
                                                   Continuous, NoFrequency);
    }

    DiscountFactor WulinYongDoubleBarrierEngine::dividendDiscount() const {
        return process_->dividendYield()->discount(residualTime());
    }

    Real WulinYongDoubleBarrierEngine::D(Real X, Real lambda, Real sigma, Real T) const {
        return (std::log(X) + lambda * T)/(sigma * std::sqrt(T));
    }

}

