
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

/*! \file dividendeuropeanoption.cpp
    \brief european option with discrete deterministic dividends

    \fullpath
    ql/Pricers/%dividendeuropeanoption.cpp
*/

// $Id$

#include "ql/Pricers/dividendeuropeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        DividendEuropeanOption::DividendEuropeanOption(
            Option::Type type, double underlying, double strike, 
            Rate dividendYield, Rate riskFreeRate, Time residualTime, 
            double volatility, const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates):
            EuropeanOption(type, underlying - riskless(riskFreeRate,
                dividends, exdivdates), strike, dividendYield,
                riskFreeRate, residualTime, volatility),
                dividends_(dividends),exDivDates_(exdivdates){

                QL_REQUIRE(dividends_.size() == exDivDates_.size(),
                    "the number of dividends is different from that of dates");
                for(size_t j = 0; j < dividends_.size(); j++){

                    QL_REQUIRE(exDivDates_[j] >= 0, "The "+
                         IntegerFormatter::toString(j)+ "-th" +
                        "dividend date is negative"    + "(" +
                        DoubleFormatter::toString(exDivDates_[j]) + ")");

                    QL_REQUIRE(exDivDates_[j] <= residualTime,"The " +
                        IntegerFormatter::toString(j) + "-th" +
                        "dividend date is greater than residual time" + "(" +
                        DoubleFormatter::toString(exDivDates_[j]) + ">" +
                        DoubleFormatter::toString(residualTime)    + ")");
                }

            }

        Handle<SingleAssetOption> DividendEuropeanOption::clone() const{
            return Handle<SingleAssetOption>(new DividendEuropeanOption(*this));
        }

        double DividendEuropeanOption::theta() const{

            double tmp_theta = EuropeanOption::theta();
            double delta_theta = 0.0;
            for(size_t j = 0; j < dividends_.size(); j++)
                delta_theta -= dividends_[j] * riskFreeRate_ *
                               QL_EXP(-riskFreeRate_ * exDivDates_[j]);
            return tmp_theta + delta_theta * EuropeanOption::delta();
        }

        double DividendEuropeanOption::rho() const{

            double tmp_rho = EuropeanOption::rho();
            double delta_rho = 0.0;
            for(size_t j = 0; j < dividends_.size(); j++)
                delta_rho += dividends_[j] * exDivDates_[j] *
                             QL_EXP(-riskFreeRate_ * exDivDates_[j]);
            return tmp_rho + delta_rho * EuropeanOption::delta();
        }

    }

}

