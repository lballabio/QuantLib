
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

/*! \file continuousgeometricapo.hpp
    \brief Continuous Geometric Average Price Option (European exercise)
*/

#ifndef quantlib_continous_geometric_average_price_option_h
#define quantlib_continous_geometric_average_price_option_h

#include <ql/Pricers/europeanoption.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    //! Continuous geometric average-price option (European exercise)
    /*! This class implements a continuous geometric average price
        asian option with european exercise.  The formula is from
        "Option Pricing Formulas", E. G. Haug (1997) pag 96-97

        \todo add Average Strike version and make it backward starting
    */
    class ContinuousGeometricAPO {
      public:
        ContinuousGeometricAPO(Option::Type type,
                               double underlying,
                               double strike,
                               Spread dividendYield,
                               Rate riskFreeRate,
                               Time residualTime,
                               double volatility);
        double value() const { return value_; }
        double delta() const { return delta_; }
        double gamma() const { return gamma_; }
        double theta() const { return theta_; }
        double vega() const { return vega_; }
        double rho() const { return rho_; }
        double dividendRho() const { return dividendRho_; }
      private:
        double value_;
        double delta_, gamma_;
        double theta_;
        double vega_;
        double rho_, dividendRho_;
    };


    // inline definitions

    inline ContinuousGeometricAPO::ContinuousGeometricAPO(
                          Option::Type type, double underlying, double strike,
                          Spread dividendYield, Rate riskFreeRate, 
                          Time residualTime, double volatility) {

        double r = riskFreeRate;
        double q = (riskFreeRate + dividendYield + 
                    volatility*volatility/6.0)/2.0;
        double sigma = volatility/QL_SQRT(3.0);

        double discount = QL_EXP(-r*residualTime);
        double qDiscount = QL_EXP(-q*residualTime);
        double forward = underlying*qDiscount/discount;
        double variance = sigma*sigma*residualTime;
        Handle<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type,strike));
        BlackFormula black(forward, discount, variance, payoff);

        value_ = black.value();
        delta_ = black.delta(underlying);
        gamma_ = black.gamma(underlying);
        theta_ = black.theta(underlying, residualTime);
        vega_ = black.vega(residualTime)/QL_SQRT(3.0) - 
                black.rho(residualTime)*sigma*sigma/4.0;
        rho_ = black.rho(residualTime)/2.0;
        dividendRho_ = black.dividendRho(residualTime);
    }

}


#endif
