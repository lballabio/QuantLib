
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

#ifndef quantlib_continous_geometric_average_price_option_hpp
#define quantlib_continous_geometric_average_price_option_hpp

#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

#ifndef QL_DISABLE_DEPRECATED

    //! Continuous geometric average-price option (European exercise)
    /*! This class implements a continuous geometric average price
        asian option with european exercise.  The formula is from
        "Option Pricing Formulas", E. G. Haug (1997) pag 96-97

        \deprecated use ContinuousAveragingAsianOption with
                    AnalyticContinuousAveragingAsianEngine instead
    */
    class ContinuousGeometricAPO {
      public:
        ContinuousGeometricAPO(Option::Type type,
                               Real underlying,
                               Real strike,
                               Spread dividendYield,
                               Rate riskFreeRate,
                               Time residualTime,
                               Volatility volatility);
        Real value() const { return value_; }
        Real delta() const { return delta_; }
        Real gamma() const { return gamma_; }
        Real theta() const { return theta_; }
        Real vega() const { return vega_; }
        Real rho() const { return rho_; }
        Real dividendRho() const { return dividendRho_; }
      private:
        Real value_;
        Real delta_, gamma_;
        Real theta_;
        Real vega_;
        Real rho_, dividendRho_;
    };


    // inline definitions

    inline ContinuousGeometricAPO::ContinuousGeometricAPO(
                          Option::Type type, Real underlying, Real strike,
                          Spread dividendYield, Rate riskFreeRate, 
                          Time residualTime, Volatility volatility) {

        Rate r = riskFreeRate;
        Rate q = (riskFreeRate + dividendYield + 
                  volatility*volatility/6.0)/2.0;
        Volatility sigma = volatility/QL_SQRT(3.0);

        DiscountFactor discount = QL_EXP(-r*residualTime);
        DiscountFactor qDiscount = QL_EXP(-q*residualTime);
        Real forward = underlying*qDiscount/discount;
        Real variance = sigma*sigma*residualTime;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                         new PlainVanillaPayoff(type,strike));
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

#endif

}


#endif
