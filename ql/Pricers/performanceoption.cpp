
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Pricers/performanceoption.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    PerformanceOption::PerformanceOption(
                       Option::Type type, double, double moneyness,
                       const std::vector<Spread>& dividendYield,
                       const std::vector<Rate>& riskFreeRate,
                       const std::vector<Time>& times,
                       const std::vector<double>& volatility) {

        QL_REQUIRE(times.size() > 0,
                   "At least one option is required for performance options");
        QL_REQUIRE(dividendYield.size()==times.size(),
                   "PerformanceOption: dividendYield vector of wrong size");
        QL_REQUIRE(riskFreeRate.size()==times.size(),
                   "PerformanceOption: riskFreeRate vector of wrong size");
        QL_REQUIRE(volatility.size()==times.size(),
                   "PerformanceOption: volatility vector of wrong size");

        double discount = QL_EXP(-riskFreeRate[0] * times[0]);

        value_ = delta_ = gamma_ = theta_ =
            rho_ = dividendRho_ = vega_ = 0.0;

        for (Size i = 1; i < times.size(); i++) {
            Time dt = times[i] - times[i-1];
            double rDiscount = QL_EXP(-riskFreeRate[i] * dt);
            double qDiscount = QL_EXP(-dividendYield[i] * dt);
            double forward = (1.0/moneyness)*qDiscount/rDiscount;
            double variance = volatility[i]*volatility[i]*dt;
            Handle<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type,1.0));
            BlackFormula black(forward, rDiscount, variance, payoff);
            value_ += discount * moneyness * black.value();
            delta_ += 0.0;
            gamma_ += 0.0;
            theta_ += riskFreeRate[i-1] * discount * moneyness * black.value();
            rho_ += discount * moneyness * 
                (black.rho(dt) - times[i-1] * black.value());
            dividendRho_ += discount * moneyness * black.dividendRho(dt);
            vega_ += discount * moneyness * black.vega(dt);
 
            discount *= QL_EXP(-riskFreeRate[i] * dt);
       }
    }

}

