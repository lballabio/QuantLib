
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
/*! \file dividendeuropeanoption.cpp
    \brief european option with discrete deterministic dividends

    \fullpath
    ql/Pricers/%dividendeuropeanoption.cpp
*/

// $Id$

#include <ql/Pricers/fddividendeuropeanoption.hpp>

namespace QuantLib {

    namespace Pricers {

        FdDividendEuropeanOption::FdDividendEuropeanOption(
            Option::Type type, double underlying, double strike,
            Spread dividendYield, Rate riskFreeRate, Time residualTime,
            double volatility, const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates):
            EuropeanOption(type, underlying - riskless(riskFreeRate,
                dividends, exdivdates), strike, dividendYield,
                riskFreeRate, residualTime, volatility),
                dividends_(dividends),exDivDates_(exdivdates){

                QL_REQUIRE(dividends_.size() == exDivDates_.size(),
                    "the number of dividends is different from that of dates");
                for(Size j = 0; j < dividends_.size(); j++){

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

        Handle<SingleAssetOption> FdDividendEuropeanOption::clone() const{
            return Handle<SingleAssetOption>(new FdDividendEuropeanOption(*this));
        }

        double FdDividendEuropeanOption::theta() const{

            double tmp_theta = EuropeanOption::theta();
            double delta_theta = 0.0;
            for(Size j = 0; j < dividends_.size(); j++)
                delta_theta -= dividends_[j] * riskFreeRate_ *
                               QL_EXP(-riskFreeRate_ * exDivDates_[j]);
            return tmp_theta + delta_theta * EuropeanOption::delta();
        }

        double FdDividendEuropeanOption::rho() const{

            double tmp_rho = EuropeanOption::rho();
            double delta_rho = 0.0;
            for(Size j = 0; j < dividends_.size(); j++)
                delta_rho += dividends_[j] * exDivDates_[j] *
                             QL_EXP(-riskFreeRate_ * exDivDates_[j]);
            return tmp_rho + delta_rho * EuropeanOption::delta();
        }

    }

}

