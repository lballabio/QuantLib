
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

            originalArgs_->type = parameters_.type;
            originalArgs_->underlying = parameters_.underlying;
            originalArgs_->strike = parameters_.strike;
            originalArgs_->dividendYield = parameters_.dividendYield +
                parameters_.riskFreeRate - parameters_.foreignRiskFreeRate +
                parameters_.correlation * parameters_.volatility *
                parameters_.exchangeRateVolatility;
            originalArgs_->riskFreeRate = parameters_.riskFreeRate;
            originalArgs_->residualTime = parameters_.residualTime;
            originalArgs_->volatility = parameters_.volatility;

            originalArgs_->validate();
            
            originalEngine_->calculate();

            results_.value = originalResults_->value;
            results_.delta = originalResults_->delta;
            results_.gamma = originalResults_->gamma;
            results_.theta = originalResults_->theta;
            results_.rho = 0.0;
            results_.dividendRho = 0.0;
            results_.vega = 0.0;

            results_.qvega = 0.0;
            results_.qrho = 0.0;
            results_.qlambda = 0.0;
        }

    }

}

