
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

#ifndef QL_DISABLE_DEPRECATED
    DividendEuropeanOption::DividendEuropeanOption(
                   Option::Type type, Real underlying, Real strike,
                   Spread dividendYield, Rate riskFreeRate, Time residualTime,
                   Volatility volatility, const std::vector<Real>& dividends,
                   const std::vector<Time>& exdivdates) {

        QL_REQUIRE(dividends.size() == exdivdates.size(),
                   "the number of dividends is different from that of dates");

        for (Size j = 0; j < dividends.size(); j++) {
            QL_REQUIRE(exdivdates[j] >= 0, "the "+
                       SizeFormatter::toOrdinal(j) +
                       " dividend time is negative"    + "(" +
                       DecimalFormatter::toString(exdivdates[j]) + ")");

            QL_REQUIRE(exdivdates[j] <= residualTime,"the " +
                       SizeFormatter::toOrdinal(j) +
                       " dividend time is greater than residual time" + "(" +
                       DecimalFormatter::toString(exdivdates[j]) + ">" +
                       DecimalFormatter::toString(residualTime)    + ")");
        }

        Real riskless = 0.0;
        Size i;
        for (i = 0; i < dividends.size(); i++)
            riskless += dividends[i]*QL_EXP(-riskFreeRate*exdivdates[i]);
        Real spot = underlying-riskless;
        DiscountFactor discount = QL_EXP(-riskFreeRate*residualTime);
        DiscountFactor qDiscount = QL_EXP(-dividendYield*residualTime);
        Real forward = spot*qDiscount/discount;
        Real variance = volatility*volatility*residualTime;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                         new PlainVanillaPayoff(type,strike));
        BlackFormula black(forward, discount, variance, payoff);

        value_ = black.value();
        delta_ = black.delta(spot);
        gamma_ = black.gamma(spot);
        vega_ = black.vega(residualTime);

        Real delta_theta = 0.0;
        for (i = 0; i < dividends.size(); i++)
            delta_theta -= dividends[i] * riskFreeRate * 
                           QL_EXP(-riskFreeRate * exdivdates[i]);
        theta_ = black.theta(spot, residualTime) +
                 delta_theta * black.delta(spot);

        Real delta_rho = 0.0;
        for (i = 0; i < dividends.size(); i++)
            delta_rho += dividends[i] * exdivdates[i] *
                         QL_EXP(-riskFreeRate * exdivdates[i]);
        rho_ = black.rho(residualTime) +
               delta_rho * black.delta(spot);
    }
#endif

}

