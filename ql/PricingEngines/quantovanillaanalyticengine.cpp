
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
#include <ql/TermStructures/quantotermstructure.hpp>

using QuantLib::TermStructures::QuantoTermStructure;

namespace QuantLib {

    namespace PricingEngines {

        void QuantoVanillaAnalyticEngine::calculate() const {

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = arguments_.underlying;
            originalArguments_->strike = arguments_.strike;
            originalArguments_->dividendTS = Handle<TermStructure>(new
                QuantoTermStructure(arguments_.dividendTS,
                arguments_.riskFreeTS, arguments_.foreignRiskFreeTS,
                arguments_.volTS, arguments_.exchRateVolTS,
                arguments_.correlation));
            originalArguments_->riskFreeTS = arguments_.riskFreeTS;
            originalArguments_->volTS = arguments_.volTS;
            originalArguments_->exerciseDate = arguments_.exerciseDate;

            originalArguments_->validate();
            originalEngine_->calculate();

            results_.value = originalResults_->value;
            results_.delta = originalResults_->delta;
            results_.gamma = originalResults_->gamma;
            results_.theta = originalResults_->theta;
            results_.rho = originalResults_->rho +
                originalResults_->dividendRho;
            results_.dividendRho = originalResults_->dividendRho;
            // exchangeRtae level needed here!!!!!
            double exchangeRateFlatVol = arguments_.exchRateVolTS->blackVol(
                arguments_.exerciseDate, arguments_.underlying);
            results_.vega = originalResults_->vega +
                arguments_.correlation * exchangeRateFlatVol *
                originalResults_->dividendRho;


            double volatility = arguments_.volTS->blackVol(
                arguments_.exerciseDate, arguments_.underlying);
            results_.qvega = + arguments_.correlation
                * arguments_.volTS->blackVol(arguments_.exerciseDate, arguments_.underlying) *
                originalResults_->dividendRho;
            results_.qrho = - originalResults_->dividendRho;
            results_.qlambda = exchangeRateFlatVol *
                volatility * originalResults_->dividendRho;
        }

    }

}

