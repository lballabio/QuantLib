
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

namespace QuantLib {

    PerformanceOption::PerformanceOption(
                       Option::Type type, double underlying, double moneyness,
                       const std::vector<Spread>& dividendYield,
                       const std::vector<Rate>& riskFreeRate,
                       const std::vector<Time>& times,
                       const std::vector<double>& volatility)
    : moneyness_(moneyness), riskFreeRate_(riskFreeRate), times_(times),
      numOptions_(times.size()), optionlet_(numOptions_),
      discounts_(numOptions_) {

        QL_REQUIRE(numOptions_ > 0,
                   "At least one option is required for performance options");
        QL_REQUIRE(dividendYield.size()==numOptions_,
                   "PerformanceOption: dividendYield vector of wrong size");
        QL_REQUIRE(riskFreeRate.size()==numOptions_,
                   "PerformanceOption: riskFreeRate vector of wrong size");
        QL_REQUIRE(volatility.size()==numOptions_,
                   "PerformanceOption: volatility vector of wrong size");

        discounts_[0] = QL_EXP(-riskFreeRate[0] * times[0]);
        double dummyStrike = underlying * moneyness_;
        optionlet_[0] = boost::shared_ptr<EuropeanOption>(
            new EuropeanOption(type,
            underlying, dummyStrike,
            dividendYield[0],
            riskFreeRate[0], times[0], volatility[0]));

        for(Size i = 1; i < numOptions_; i++) {
            discounts_[i] = discounts_[i-1]*
                QL_EXP(-riskFreeRate[i] * (times[i] - times[i-1]));
            optionlet_[i] = boost::shared_ptr<EuropeanOption>(
                new EuropeanOption(type,
                1.0/moneyness, 1.0,
                dividendYield[i],
                riskFreeRate[i], times[i] - times[i-1], volatility[i]));
        }
    }

    double PerformanceOption::value() const {
        double result = 0.0 * optionlet_[0] -> value();
        for(Size i = 1; i < numOptions_; i++)
            result += discounts_[i-1] * 
                moneyness_ * optionlet_[i] -> value();
        return result;
    }

    double PerformanceOption::delta() const {
        double result = 0.0 * 
            moneyness_ * optionlet_[0] -> delta();
        return result;
    }

    double PerformanceOption::gamma() const {
        double result = 0.0 * 
            moneyness_ * optionlet_[0] -> gamma();
        return result;
    }

    double PerformanceOption::theta() const {
        double result = 0.0*optionlet_[0] -> theta();
        for(Size i = 1; i < numOptions_; i++)
            result += riskFreeRate_[i-1] * discounts_[i-1] * 
                moneyness_ * optionlet_[i] -> value();
        return result;
    }

    double PerformanceOption::rho() const {
        double result = 0.0*optionlet_[0] -> rho();
        for(Size i = 1; i < numOptions_; i++)
            result += discounts_[i-1] * 
                moneyness_ * (optionlet_[i] -> rho()
                              - times_[i-1] * optionlet_[i] -> value());
        return result;
    }

    double PerformanceOption::dividendRho() const {
        double result = 0.0*optionlet_[0] -> dividendRho();
        for(Size i = 1; i < numOptions_; i++)
            result += discounts_[i-1] * 
                moneyness_ * optionlet_[i] -> dividendRho();
        return result;
    }

    double PerformanceOption::vega() const {
        double result = 0.0*optionlet_[0] -> vega();
        for(Size i = 1; i < numOptions_; i++)
            result += discounts_[i-1] *
                moneyness_ * optionlet_[i] -> vega();
        return result;
    }

}







