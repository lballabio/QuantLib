
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
/*! \file quantovanillaengine.cpp
    \brief Quanto vanilla engine using analytic formulas

    \fullpath
    ql/Pricers/%quantovanillaengine.cpp
*/

// $Id$

#include <ql/PricingEngines/quantoengines.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void QuantoVanillaAnalyticEngine::calculate() const {

            originalParameters_->type = parameters_.type;
            originalParameters_->underlying = parameters_.underlying;
            originalParameters_->strike = parameters_.strike;
            originalParameters_->dividendYield = parameters_.dividendYield +
                parameters_.riskFreeRate - parameters_.foreignRiskFreeRate +
                parameters_.correlation * parameters_.volatility *
                parameters_.exchangeRateVolatility;
            originalParameters_->riskFreeRate = parameters_.riskFreeRate;
            originalParameters_->residualTime = parameters_.residualTime;
            originalParameters_->volatility = parameters_.volatility;

            originalParameters_->validate();

            originalEngine_->calculate();

            results_.value = originalResults_->value;
            results_.delta = originalResults_->delta;
            results_.gamma = originalResults_->gamma;
            results_.theta = originalResults_->theta;
            results_.rho = originalResults_->rho +
                originalResults_->dividendRho;
            results_.dividendRho = originalResults_->dividendRho;
            results_.vega = originalResults_->vega +
                parameters_.correlation * parameters_.exchangeRateVolatility *
                originalResults_->dividendRho ;

            results_.qvega = parameters_.correlation * parameters_.volatility *
                originalResults_->dividendRho;
            results_.qrho = - originalResults_->dividendRho;
            results_.qlambda = parameters_.exchangeRateVolatility *
                parameters_.volatility * originalResults_->dividendRho;
        }

    }

}

