
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

#include <ql/Pricers/dividendeuropeanoption.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    DividendEuropeanOption::DividendEuropeanOption(
                   Option::Type type, double underlying, double strike,
                   Spread dividendYield, Rate riskFreeRate, Time residualTime,
                   double volatility, const std::vector<double>& dividends,
                   const std::vector<Time>& exdivdates) {

        QL_REQUIRE(dividends.size() == exdivdates.size(),
                   "the number of dividends is different from that of dates");

        for (Size j = 0; j < dividends.size(); j++) {
            QL_REQUIRE(exdivdates[j] >= 0, "The "+
                       SizeFormatter::toOrdinal(j) +
                       " dividend time is negative"    + "(" +
                       DoubleFormatter::toString(exdivdates[j]) + ")");

            QL_REQUIRE(exdivdates[j] <= residualTime,"The " +
                       SizeFormatter::toOrdinal(j) +
                       " dividend time is greater than residual time" + "(" +
                       DoubleFormatter::toString(exdivdates[j]) + ">" +
                       DoubleFormatter::toString(residualTime)    + ")");
        }

        double riskless = 0.0;
        Size i;
        for (i = 0; i < dividends.size(); i++)
            riskless += dividends[i]*QL_EXP(-riskFreeRate*exdivdates[i]);
        double spot = underlying-riskless;
        double discount = QL_EXP(-riskFreeRate*residualTime);
        double qDiscount = QL_EXP(-dividendYield*residualTime);
        double forward = spot*qDiscount/discount;
        double variance = volatility*volatility*residualTime;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                         new PlainVanillaPayoff(type,strike));
        BlackFormula black(forward, discount, variance, payoff);

        value_ = black.value();
        delta_ = black.delta(spot);
        gamma_ = black.gamma(spot);
        vega_ = black.vega(residualTime);

        double delta_theta = 0.0;
        for (i = 0; i < dividends.size(); i++)
            delta_theta -= dividends[i] * riskFreeRate * 
                           QL_EXP(-riskFreeRate * exdivdates[i]);
        theta_ = black.theta(spot, residualTime) +
                 delta_theta * black.delta(spot);

        double delta_rho = 0.0;
        for (i = 0; i < dividends.size(); i++)
            delta_rho += dividends[i] * exdivdates[i] *
                         QL_EXP(-riskFreeRate * exdivdates[i]);
        rho_ = black.rho(residualTime) +
               delta_rho * black.delta(spot);
    }

}

