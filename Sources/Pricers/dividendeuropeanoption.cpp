
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file dividendeuropeanoption.cpp
    \brief european option with discrete deterministic dividends

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/dividendeuropeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        DividendEuropeanOption::DividendEuropeanOption(
            Type type, double underlying, double strike, Rate dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates):
            dividends_(dividends),exDivDates_(exdivdates),
            BSMEuropeanOption(type, underlying - riskless(riskFreeRate,
                dividends, exdivdates), strike, dividendYield,
                riskFreeRate, residualTime, volatility){

                QL_REQUIRE(dividends_.size() == exDivDates_.size(),
                    "the number of dividends is different from that of dates");
                for(int j = 0; j < dividends_.size(); j++){

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

        Handle<BSMOption> DividendEuropeanOption::clone() const{
            return Handle<BSMOption>(new DividendEuropeanOption(*this));
        }

        double DividendEuropeanOption::theta() const{

            double tmp_theta = BSMEuropeanOption::theta();
            double delta_theta = 0.0;
            for(int j = 0; j < dividends_.size(); j++)
                delta_theta -= dividends_[j] * riskFreeRate_ *
                               QL_EXP(-riskFreeRate_ * exDivDates_[j]);
            return tmp_theta + delta_theta * BSMEuropeanOption::delta();
        }

        double DividendEuropeanOption::rho() const{

            double tmp_rho = BSMEuropeanOption::rho();
            double delta_rho = 0.0;
            for(int j = 0; j < dividends_.size(); j++)
                delta_rho += dividends_[j] * exDivDates_[j] *
                             QL_EXP(-riskFreeRate_ * exDivDates_[j]);
            return tmp_rho + delta_rho * BSMEuropeanOption::delta();
        }

    }

}

