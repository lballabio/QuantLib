
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
/*! \file quantovanillaanalyticengine.cpp
    \brief Quanto vanilla engine using analytic formulas

    \fullpath
    ql/Pricers/%quantovanillaanalyticengine.cpp
*/

// $Id$

#include <ql/PricingEngines/quantoengines.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void QuantoVanillaAnalyticEngine::calculate() const {

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = arguments_.underlying;
            originalArguments_->strike = arguments_.strike;
            originalArguments_->dividendYield = arguments_.dividendYield
                + arguments_.riskFreeRate - arguments_.foreignRiskFreeRate
                + arguments_.correlation *
                arguments_.volatility * arguments_.exchangeRateVolatility;
            originalArguments_->riskFreeRate = arguments_.riskFreeRate;
            originalArguments_->residualTime = arguments_.residualTime;
            originalArguments_->volatility = arguments_.volatility;

            originalArguments_->validate();

            originalEngine_->calculate();

            results_.value = originalResults_->value;
            results_.delta = originalResults_->delta;
            results_.gamma = originalResults_->gamma;
            results_.theta = originalResults_->theta;
            results_.rho = originalResults_->rho +
                originalResults_->dividendRho;
            results_.dividendRho = originalResults_->dividendRho;
            results_.vega = originalResults_->vega +
                arguments_.correlation * arguments_.exchangeRateVolatility *
                originalResults_->dividendRho;

            results_.qvega = + arguments_.correlation
                * arguments_.volatility *
                originalResults_->dividendRho;
            results_.qrho = - originalResults_->dividendRho;
            results_.qlambda = arguments_.exchangeRateVolatility *
                arguments_.volatility * originalResults_->dividendRho;
        }

    }

}

