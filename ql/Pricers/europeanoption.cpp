
/*
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

#include <ql/Pricers/europeanoption.hpp>

namespace QuantLib {

    #if !defined(QL_PATCH_SOLARIS)
    const CumulativeNormalDistribution EuropeanOption::f_;
    #endif

    EuropeanOption::EuropeanOption(Option::Type type, double underlying,
                                   double strike, Spread dividendYield, 
                                   Rate riskFreeRate, Time residualTime, 
                                   double volatility)
    : SingleAssetOption(type, underlying, strike, dividendYield,
                        riskFreeRate, residualTime, volatility),
      alpha_(Null<double>()), beta_(Null<double>()),
      standardDeviation_(Null<double>()), D1_(Null<double>()),
      D2_(Null<double>()), NID1_(Null<double>()),
      dividendDiscount_(Null<DiscountFactor>()),
      riskFreeDiscount_(Null<DiscountFactor>()) {}

    void EuropeanOption::setVolatility(double newVolatility) {
        SingleAssetOption::setVolatility(newVolatility);
        D1_ = D2_ = alpha_ = beta_ = NID1_ = Null<double>();
        standardDeviation_ = Null<double>();
    }

    void EuropeanOption::setRiskFreeRate(Rate newRate) {
        SingleAssetOption::setRiskFreeRate(newRate);
        D1_ = D2_ = alpha_ = beta_ = NID1_ = Null<double>();
        riskFreeDiscount_ = Null<DiscountFactor>();
    }

    void EuropeanOption::setDividendYield(Rate newDividendYield) {
        SingleAssetOption::setDividendYield(newDividendYield);
        D1_ = D2_ = alpha_ = beta_ = NID1_ = Null<double>();
        dividendDiscount_ = Null<DiscountFactor>();
    }

    double EuropeanOption::value() const {
        return  underlying_ * dividendDiscount() * alpha() -
            payoff_.strike() * riskFreeDiscount() * beta();
    }

    double EuropeanOption::delta() const {
        return dividendDiscount()*alpha();
    }

    double EuropeanOption::gamma() const {
        return NID1()*dividendDiscount()/(underlying_*standardDeviation());
    }

    double EuropeanOption::theta() const {
        return -underlying_ * NID1() * volatility_ *
            dividendDiscount()/(2.0*QL_SQRT(residualTime_)) +
            dividendYield_*underlying_*alpha()*dividendDiscount() -
            riskFreeRate_*payoff_.strike()*riskFreeDiscount()*beta();
    }

    double EuropeanOption::rho() const {
        return residualTime_*riskFreeDiscount()*payoff_.strike()*beta();
    }

    double EuropeanOption::dividendRho() const {
        return -residualTime_*dividendDiscount()*underlying_*alpha();
    }

    double EuropeanOption::vega() const {
        return underlying_*NID1()*dividendDiscount()*QL_SQRT(residualTime_);
    }

}
