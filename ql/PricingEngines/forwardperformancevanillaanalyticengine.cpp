
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file forwardperformancevanillaanalyticengine.cpp
    \brief Forward Performance vanilla engine using analytic formulas

    \fullpath
    ql/Pricers/%forwardperformancevanillaanalyticengine.cpp
*/

// $Id$

#include <ql/PricingEngines/forwardengines.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void ForwardPerformanceVanillaAnalyticEngine::calculate() const {

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = 1.0;
            originalArguments_->strike = arguments_.moneyness;
            originalArguments_->dividendYield = arguments_.dividendYield;
            originalArguments_->riskFreeRate = arguments_.riskFreeRate;
            originalArguments_->residualTime = arguments_.residualTime
                - arguments_.resetTime;
            originalArguments_->volatility = arguments_.volatility;

            originalArguments_->validate();
            originalEngine_->calculate();

            double discR = QL_EXP(-arguments_.riskFreeRate *
                arguments_.resetTime);

            results_.value = discR * originalResults_->value;
            results_.delta = 0.0;
            results_.gamma = 0.0;
            results_.theta = arguments_.riskFreeRate * results_.value;
            results_.vega = discR * originalResults_->vega;
            results_.rho = - arguments_.resetTime * results_.value +
                discR * originalResults_->rho;
            results_.dividendRho = discR * originalResults_->dividendRho;

        }

    }

}

