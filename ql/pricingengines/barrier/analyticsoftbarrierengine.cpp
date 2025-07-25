/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

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
#include <ql/pricingengines/barrier/analyticsoftbarrierengine.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <utility>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <iostream>


namespace QuantLib {

    AnalyticSoftBarrierEngine::AnalyticSoftBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }


    void AnalyticSoftBarrierEngine::calculate() const {

        // Market data
        Real S = underlying();
        Real X = strike();
        Rate r = riskFreeRate();
        Rate q = dividendYield();
        Volatility sigma = volatility();

        // Barrier parameters
        Real U = barrierHi();
        Real L = barrierLo();
        SoftBarrier::Type barrierType = arguments_.barrierType;

        // Option parameters
        Time T = residualTime();
        ext::shared_ptr<PlainVanillaPayoff> payoff = ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff); 
        Option::Type optionType = payoff->optionType();
        Integer eta = (optionType == Option::Call ? 1 : -1);
        Rate b = r - q; // cost of carry


        // validate inputs 
        validateInputs(S, X, r, q, T, U, L, optionType, barrierType, sigma);

        // Edge Case: already knocked in/out 
        bool isKnockedIn = (barrierType == SoftBarrier::DownIn && S <= L) || 
                          (barrierType == SoftBarrier::UpIn && S >= U);
        bool isKnockedOut = (barrierType == SoftBarrier::DownOut && S <= L) || 
                          (barrierType == SoftBarrier::UpOut && S >= U);

        if (isKnockedIn) {
              results_.value = vanillaEquivalent();
              return;
            }

        else if (isKnockedOut) {   
            results_.value = 0.0;  
            return;
            }

        // soft barrier pricing logic
        Real w = knockInValue(S, X, r, sigma, T, U, L, b, optionType,eta);
        results_.value = (barrierType == SoftBarrier::DownIn || barrierType == SoftBarrier::UpIn)
            ? w                     // knock in price
            : vanillaEquivalent() - w;  // knock out price
        }
    

    // Implements the formula to calculate 'w' from the Haug textbook, used in soft barrier pricing
    Real AnalyticSoftBarrierEngine::knockInValue(Real S, Real X, Rate r, Volatility sigma, Time T,
                                           Real U, Real L, Real b, Option::Type optionType, 
                                           Integer eta) const {
        
        // Constants                                     
        const Real mu = (b + 0.5 * sigma * sigma) / (sigma * sigma);
        const Real sqrtT = std::sqrt(T);
        const Real lambda1 = std::exp(-0.5 * sigma * sigma * T * (mu + 0.5) * (mu - 0.5));
        const Real lambda2 = std::exp(-0.5 * sigma * sigma * T * (mu - 0.5) * (mu - 1.5));
        const Real SX = S * X;
        const Real logU2_SX = std::log((U * U) / SX);
        const Real logL2_SX = std::log((L * L) / SX);
        
        // d and e terms
        const Real d1 = logU2_SX / (sigma * sqrtT) + mu * sigma * sqrtT;
        const Real d2 = d1 - (mu + 0.5) * sigma * sqrtT;
        const Real d3 = logU2_SX / (sigma * sqrtT) + (mu - 1) * sigma * sqrtT;
        const Real d4 = d3 - (mu - 0.5) * sigma * sqrtT;
        const Real e1 = logL2_SX / (sigma * sqrtT) + mu * sigma * sqrtT;
        const Real e2 = e1 - (mu + 0.5) * sigma * sqrtT;
        const Real e3 = logL2_SX / (sigma * sqrtT) + (mu - 1) * sigma * sqrtT;
        const Real e4 = e3 - (mu - 0.5) * sigma * sqrtT;

        // cumulative normal values                                        
        const Real Nd1 = f_(eta * d1);
        const Real Nd2 = f_(eta * d2);
        const Real Nd3 = f_(eta * d3);
        const Real Nd4 = f_(eta * d4);
        const Real Ne1 = f_(eta * e1);
        const Real Ne2 = f_(eta * e2);
        const Real Ne3 = f_(eta * e3);
        const Real Ne4 = f_(eta * e4); 

        // term 1                                        
        Real term1 = eta * S * std::exp((b - r) * T) * std::pow(S, -2.0 * mu)
            * std::pow(SX, mu + 0.5) / (2.0 * (mu + 0.5));
        
        term1 *= std::pow(U * U / SX, mu + 0.5) * Nd1 - lambda1 * Nd2
               - std::pow(L * L / SX, mu + 0.5) * Ne1 + lambda1 * Ne2;
        
        // term2                                     
        Real term2 = eta * X * std::exp(-r * T) * std::pow(S, -2.0 * (mu - 1))
            * std::pow(SX, mu - 0.5) / (2.0 * (mu - 0.5));
        
        term2 *= std::pow(U * U / SX, mu - 0.5) * Nd3 - lambda2 * Nd4
               - std::pow(L * L / SX, mu - 0.5) * Ne3 + lambda2 * Ne4;
        
        // return 'w' value
        return (1.0 / (U - L)) * (term1 - term2);
    }

    // helper function to check inputs are reasonable
    void AnalyticSoftBarrierEngine::validateInputs(Real S, Real X, Rate r, Rate q, Time T, Real U, Real L,
                                                   Option::Type optionType, SoftBarrier::Type barrierType,
                                                   Real sigma) const {
        // Core Parameter checks                                                
        QL_REQUIRE(S > 0.0, "Spot price must be > 0");
        QL_REQUIRE(X > 0.0, "Strike price must be > 0");
        QL_REQUIRE(T > 0.0, "Option must have time to maturity > 0");
        QL_REQUIRE(U > 0.0 && L > 0.0, "Barrier levels must be positive");
        QL_REQUIRE(U >= L, "Upper barrier must be greater than or equal to lower barrier");
        QL_REQUIRE(optionType == Option::Call || optionType == Option::Put, "Invalid option type");                                       
              
        // Bounds checks
        QL_REQUIRE(r <= 1.0 && r >= -0.05, "Interest rate must be between -5% and 100%");
        QL_REQUIRE(q <= 1.0 && q >= -0.1, "Dividend yield must be between -10% and 100%");
        QL_REQUIRE(std::fabs(r - q) >= 1e-4, "r and q too close leads to numerical instability"); 
        
        // volatility warning
        if (sigma <= 0.02){
            std::cerr << "Warning: Sigma <= 0.02 -> may cause unstable results" << std::endl;  
        }

        
        // Barrier type checks
        switch (optionType){
            case Option::Call:
                QL_REQUIRE(barrierType == SoftBarrier::DownIn || barrierType == SoftBarrier::DownOut,
                        "Only 'down' barriers supported for soft call options");

                if (barrierType == SoftBarrier::DownIn && S < L){
                    std::cerr << "Warning: Spot price is below the lower barrier."
                            << "This soft barrier option is already knocked in and will be priced as vanilla."
                            << "For optimal performance, use 'VanillaOption' directly for this case." << std::endl; 
                }
                break;

            case Option::Put:
                QL_REQUIRE(barrierType == SoftBarrier::UpIn || barrierType == SoftBarrier::UpOut,
                        "Only up barriers supported for soft put options");

                if (barrierType == SoftBarrier::UpIn && S > U){
                    std::cerr << "Warning: Spot price is above the upper barrier."
                            << "This soft barrier option is already knocked in and will be priced as vanilla."
                            << "For optimal performance, use 'VanillaOption' directly for this case." << std::endl;
                }
        }}
    

    /// helper functions 
    Real AnalyticSoftBarrierEngine::underlying() const {
        return process_->x0();
    }

    Real AnalyticSoftBarrierEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff = ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);  
        QL_REQUIRE(payoff, "non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticSoftBarrierEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Volatility AnalyticSoftBarrierEngine::volatility() const {
        return process_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Real AnalyticSoftBarrierEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Real AnalyticSoftBarrierEngine::barrierLo() const {
        return arguments_.barrier_lo;
    }

    Real AnalyticSoftBarrierEngine::barrierHi() const {
        return arguments_.barrier_hi;
    }

    Rate AnalyticSoftBarrierEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous, NoFrequency);
    }

    DiscountFactor AnalyticSoftBarrierEngine::riskFreeDiscount() const {
        return process_->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticSoftBarrierEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(residualTime(),Continuous, NoFrequency);
    }

    DiscountFactor AnalyticSoftBarrierEngine::dividendDiscount() const {
        return process_->dividendYield()->discount(residualTime());
    }
            

    Real AnalyticSoftBarrierEngine::vanillaEquivalent() const {
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        Real forwardPrice = underlying() * dividendDiscount() / riskFreeDiscount();
        BlackCalculator black(payoff, forwardPrice, stdDeviation(), riskFreeDiscount());
        Real vanilla = black.value();
        return std::max(vanilla, 0.0);

}       
}

