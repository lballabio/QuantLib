
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file dividendamericanoption.cpp
    \brief american option with discrete deterministic dividends

    $Id$
*/

// $Source$
// $Log$
// Revision 1.30  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.29  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.28  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/dividendamericanoption.hpp"

namespace QuantLib {

    namespace Pricers {
         DividendAmericanOption::DividendAmericanOption(Type type,
            double underlying, double strike, Rate dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates,
            int timeSteps, int gridPoints)
         :DividendOption(type, underlying, strike, dividendYield,
            riskFreeRate, residualTime, volatility, dividends,
            exdivdates, timeSteps, gridPoints){}

        Handle<SingleAssetOption> DividendAmericanOption::clone() const {
            return Handle<SingleAssetOption>(new DividendAmericanOption(*this));
        }

    }

}
