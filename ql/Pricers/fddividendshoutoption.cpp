
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fddividendshoutoption.cpp
    \brief base class for shout options with dividends
*/

#include <ql/Pricers/fddividendshoutoption.hpp>

namespace QuantLib {

    namespace Pricers {

        FdDividendShoutOption::FdDividendShoutOption(Option::Type type,
                double underlying, double strike, Spread dividendYield,
                Rate riskFreeRate, Time residualTime, double volatility,
                const std::vector<double>& dividends,
                const std::vector<Time>& exdivdates,
                int timeSteps, int gridPoints)
       : FdDividendOption(type, underlying, strike, dividendYield,
                        riskFreeRate, residualTime, volatility,
                        dividends, exdivdates, timeSteps, gridPoints){}

        void FdDividendShoutOption::initializeStepCondition() const {
            stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                new FiniteDifferences::ShoutCondition(intrinsicValues_,
                residualTime_, riskFreeRate_));
        }

        Handle<SingleAssetOption> FdDividendShoutOption::clone() const {
            return Handle<SingleAssetOption>(new FdDividendShoutOption(*this));
        }

    }

}
