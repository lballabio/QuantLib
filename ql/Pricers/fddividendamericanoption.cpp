
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file dividendamericanoption.cpp
    \brief american option with discrete deterministic dividends

    \fullpath
    ql/Pricers/%dividendamericanoption.cpp
*/

// $Id$

#include <ql/Pricers/fddividendamericanoption.hpp>

namespace QuantLib {

    namespace Pricers {
         FdDividendAmericanOption::FdDividendAmericanOption(Option::Type type,
            double underlying, double strike, Spread dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates,
            int timeSteps, int gridPoints)
         :FdDividendOption(type, underlying, strike, dividendYield,
            riskFreeRate, residualTime, volatility, dividends,
            exdivdates, timeSteps, gridPoints){}

        Handle<SingleAssetOption> FdDividendAmericanOption::clone() const {
            return Handle<SingleAssetOption>(new FdDividendAmericanOption(*this));
        }

    }

}
