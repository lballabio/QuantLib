
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Pricers/cliquetoptionpricer.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

#ifndef QL_DISABLE_DEPRECATED

    CliquetOptionPricer::CliquetOptionPricer(
                                 Option::Type type,
                                 double underlying, double moneyness,
                                 const std::vector<Spread>& dividendYield,
                                 const std::vector<Rate>& riskFreeRate,
                                 const std::vector<Time>& times,
                                 const std::vector<double>& volatility) {

        QL_REQUIRE(times.size() > 0,
                   "at least one option is required for cliquet options");
        QL_REQUIRE(dividendYield.size() == times.size(),
                   "dividendYield vector of wrong size");
        QL_REQUIRE(riskFreeRate.size() == times.size(),
                   "riskFreeRate vector of wrong size");
        QL_REQUIRE(volatility.size() == times.size(),
                   "volatility vector of wrong size");

        double weight = 0.0;
        double discount = QL_EXP(-riskFreeRate[0]*times[0]);
        double qDiscount = QL_EXP(-dividendYield[0]*times[0]);
        double forward = underlying*qDiscount/discount;
        double variance = volatility[0]*volatility[0]*times[0];
        boost::shared_ptr<StrikedTypePayoff> payoff(
                          new PlainVanillaPayoff(type,underlying*moneyness));
        BlackFormula black(forward, discount, variance, payoff);
        value_ = weight * black.value();
        delta_ = weight * black.delta(underlying);
        gamma_ = weight * black.gamma(underlying);
        theta_ = weight * black.theta(underlying, times[0]);
        rho_ = weight * black.rho(times[0]);
        dividendRho_ = weight * black.dividendRho(times[0]);
        vega_ = weight * black.vega(times[0]);

        for (Size i = 1; i < times.size(); i++) {
            if (i == 1)
                weight = QL_EXP(-dividendYield[0] * times[0]);
            else
                weight *= QL_EXP(-dividendYield[i-1]*(times[i-1]-times[i-2]));
            Time dt = times[i] - times[i-1];
            discount = QL_EXP(-riskFreeRate[i] * dt);
            qDiscount = QL_EXP(-dividendYield[i] * dt);
            forward = underlying*qDiscount/discount;
            variance = volatility[i]*volatility[i]*dt;
            BlackFormula black(forward, discount, variance, payoff);
            value_ += weight * black.value();
            delta_ += weight * (black.delta(underlying) +
                                moneyness * discount * black.beta());
            gamma_ += 0.0;
            theta_ += dividendYield[i-1] * weight * black.value();
            rho_ += weight * black.rho(dt);
            dividendRho_ += weight * (black.dividendRho(dt) -
                                      times[i-1] * black.value());
            vega_ += weight * black.vega(dt);
        }
    }

#endif

}

