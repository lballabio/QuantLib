
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
/*! \file bermudanoption.cpp
    \brief Finite-difference evaluation of Bermudan option

    \fullpath
    ql/Pricers/%bermudanoption.cpp
*/

// $Id$

#include <ql/Pricers/fdbermudanoption.hpp>
#include <ql/Pricers/europeanoption.hpp>

namespace QuantLib {

    namespace Pricers {

        FdBermudanOption::FdBermudanOption(Option::Type type, double underlying,
            double strike, Spread dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility,
            const std::vector<Time>& dates,
            int timeSteps, int gridPoints)
        : FdMultiPeriodOption(type, underlying, strike, dividendYield,
          riskFreeRate, residualTime, volatility, gridPoints, dates,
          timeSteps) {}

        using FiniteDifferences::StandardStepCondition;

        void FdBermudanOption::initializeStepCondition() const{
            stepCondition_ = Handle<StandardStepCondition> ();
        }

        void FdBermudanOption::executeIntermediateStep(int ) const{

            int size = initialPrices_.size();
            for(int j = 0; j < size; j++)
                prices_[j] = QL_MAX(prices_[j], initialPrices_[j]);
        }

        Handle<SingleAssetOption> FdBermudanOption::clone() const {
            return Handle<SingleAssetOption>(new FdBermudanOption(type_,
                underlying_, strike_, dividendYield_, riskFreeRate_,
                residualTime_, volatility_, dates_, timeStepPerPeriod_,
                gridPoints_));
        }

    }

}
