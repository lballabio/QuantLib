
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

#include <ql/Pricers/fddividendamericanoption.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    FdDividendAmericanOption::FdDividendAmericanOption(
                         Option::Type type, Real underlying, Real strike,
                         Spread dividendYield, Rate riskFreeRate,
                         Time residualTime, Volatility volatility,
                         const std::vector<Real>& dividends,
                         const std::vector<Time>& exdivdates,
                         Size timeSteps, Size gridPoints)
    : FdDividendOption(type, underlying, strike, dividendYield,
                      riskFreeRate, residualTime, volatility, dividends,
                      exdivdates, timeSteps, gridPoints){}

    boost::shared_ptr<SingleAssetOption>
    FdDividendAmericanOption::clone() const {
        return boost::shared_ptr<SingleAssetOption>(
                                         new FdDividendAmericanOption(*this));
    }

    #endif

}
