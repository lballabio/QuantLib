
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
/*! \file cliquetoption.cpp
    \brief Textbook example of european-style multi-period option.

    \fullpath
    ql/Pricers/%cliquetoption.cpp
*/

// $Id$

#include <ql/Pricers/cliquetoption.hpp>

namespace QuantLib
{
    namespace Pricers
    {
        CliquetOption::CliquetOption(Option::Type type,
                                     double underlying,
                                     Spread dividendYield,
                                     Rate riskFreeRate,
                                     const std::vector<Time> &dates,
                                     double volatility)
        : SingleAssetOption(type, underlying, underlying, dividendYield,
                    riskFreeRate, dates[dates.size()-1], volatility),
        numPeriods_(dates.size()-1),
        optionlet_(numPeriods_),
        weight_(numPeriods_){

            QL_REQUIRE(numPeriods_ >= 1,
                       "At least two dates are required for cliquet options");

            for(int i = 0; i < numPeriods_; i++){
                weight_[i] = QL_EXP(dividendYield * dates[i]);
                optionlet_[i] = Handle<EuropeanOption>(
                    new EuropeanOption(type,
                                          underlying,
                                          underlying,
                                          dividendYield,
                                          riskFreeRate,
                                          dates[i+1] - dates[i],
                                          volatility));
            }

        }

        Handle<SingleAssetOption> CliquetOption::clone() const {
            return Handle<SingleAssetOption>(new CliquetOption(*this));
        }

        double CliquetOption::value() const {
            double optValue = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optValue += weight_[i] * optionlet_[i] -> value();
            return optValue;
        }

        double CliquetOption::delta() const {
            double optDelta = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optDelta += weight_[i] * optionlet_[i] -> delta();
            return optDelta;
        }

        double CliquetOption::gamma() const {
            double optGamma = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optGamma += weight_[i] * optionlet_[i] -> gamma();
            return optGamma;
        }

        double CliquetOption::theta() const {
            double optTheta = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optTheta += weight_[i] * optionlet_[i] -> theta();
            return optTheta;
        }

        double CliquetOption::rho() const {
            double optRho = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optRho += weight_[i] * optionlet_[i] -> rho();
            return optRho;
        }

        double CliquetOption::vega() const {
            double optVega = 0.0;
            for(int i = 0; i < numPeriods_; i++)
                optVega += weight_[i] * optionlet_[i] -> vega();
            return optVega;
        }

    }

}







