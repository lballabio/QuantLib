
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

#include <ql/Pricers/cliquetoption.hpp>

namespace QuantLib {

    CliquetOptionPricer::CliquetOptionPricer(
                                 Option::Type type,
                                 double underlying, double moneyness,
                                 const std::vector<Spread>& dividendYield,
                                 const std::vector<Rate>& riskFreeRate,
                                 const std::vector<Time>& times,
                                 const std::vector<double>& volatility)
    : moneyness_(moneyness), times_(times), dividendYield_(dividendYield), 
      numOptions_(times.size()), optionlet_(numOptions_),
      weight_(numOptions_), forwardDiscounts_(numOptions_) {

        QL_REQUIRE(numOptions_ > 0,
                   "At least one option is required for cliquet options");
        QL_REQUIRE(dividendYield.size()==numOptions_,
                   "CliquetOptionPricer: dividendYield vector of wrong size");
        QL_REQUIRE(riskFreeRate.size()==numOptions_,
                   "CliquetOptionPricer: riskFreeRate vector of wrong size");
        QL_REQUIRE(volatility.size()==numOptions_,
                   "CliquetOptionPricer: volatility vector of wrong size");

        // standard option alive at t==0 expiring at times_[0]
        weight_[0] = 0.0;
        // dividend yield discounting
        if (numOptions_>1)
            weight_[1] = QL_EXP(-dividendYield[0] * times[0]);
        Size i;
        for (i = 2; i<numOptions_; i++) {
            weight_[i] = weight_[i-1] * QL_EXP(-dividendYield[i-1] *
                                               (times[i-1]-times[i-2]));
        }


        forwardDiscounts_[0] = QL_EXP(-riskFreeRate[0] * times[0]);
        double dummyStrike = underlying * moneyness_;
        optionlet_[0] = Handle<EuropeanOption>(
            new EuropeanOption(type, underlying, dummyStrike, dividendYield[0],
                               riskFreeRate[0], times[0], volatility[0]));

        for(i = 1; i < numOptions_; i++) {
            forwardDiscounts_[i] = QL_EXP(-riskFreeRate[i] *
                                          (times[i] - times[i-1]));
            optionlet_[i] = Handle<EuropeanOption>(
                new EuropeanOption(type, underlying, underlying * moneyness_,
                                   dividendYield[i], riskFreeRate[i], 
                                   times[i] - times[i-1], volatility[i]));
        }

    }

    double CliquetOptionPricer::value() const {
        double result = weight_[0] * optionlet_[0] -> value();
        for(Size i = 1; i < numOptions_; i++)
            result += weight_[i] * optionlet_[i] -> value();
        return result;
    }

    double CliquetOptionPricer::delta() const {
        double result = weight_[0] * optionlet_[0] -> delta();
        for(Size i = 1; i < numOptions_; i++)
            result += weight_[i] *
                (optionlet_[i] -> delta() - moneyness_ *
                 forwardDiscounts_[i] *
                 optionlet_[i] -> beta());
        return result;
    }

    double CliquetOptionPricer::gamma() const {
        double result = weight_[0] * optionlet_[0] -> gamma();
        // other options have zero gamma
        return result;
    }

    double CliquetOptionPricer::theta() const {
        double result = weight_[0] * optionlet_[0] -> theta();
        for(Size i = 1; i < numOptions_; i++)
            result += dividendYield_[i-1] * weight_[i] *
                optionlet_[i] -> value();
        return result;
    }

    double CliquetOptionPricer::rho() const {
        double result = weight_[0] * optionlet_[0] -> rho();
        for(Size i = 1; i < numOptions_; i++)
            result += weight_[i] * optionlet_[i] -> rho();
        return result;
    }

    double CliquetOptionPricer::dividendRho() const {
        double result = weight_[0] * optionlet_[0] -> dividendRho();
        for(Size i = 1; i < numOptions_; i++)
            result += weight_[i] *
                (optionlet_[i] -> dividendRho()
                 - times_[i-1] * optionlet_[i] -> value());
        return result;
    }

    double CliquetOptionPricer::vega() const {
        double result = weight_[0] * optionlet_[0] -> vega();
        for(Size i = 1; i < numOptions_; i++)
            result += weight_[i] * optionlet_[i] -> vega();
        return result;
    }

}







