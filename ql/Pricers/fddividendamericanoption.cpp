
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
