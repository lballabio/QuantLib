
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

/*! \file forwardperformancevanillaanalyticengine.cpp
    \brief Forward Performance vanilla engine using analytic formulas

    \fullpath
    ql/Pricers/%forwardperformancevanillaanalyticengine.cpp
*/

// $Id$

#include <ql/PricingEngines/forwardengines.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>

using QuantLib::TermStructures::ImpliedTermStructure;
using QuantLib::VolTermStructures::BlackConstantVol;

namespace QuantLib {

    namespace PricingEngines {

        void ForwardPerformanceVanillaAnalyticEngine::calculate() const {

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = 1.0;
            originalArguments_->strike = arguments_.moneyness;
            originalArguments_->dividendTS = RelinkableHandle<TermStructure>(
                Handle<TermStructure>(
                    new ImpliedTermStructure(arguments_.dividendTS,
                                             arguments_.resetDate, 
                                             arguments_.resetDate)));
            originalArguments_->riskFreeTS = RelinkableHandle<TermStructure>(
                Handle<TermStructure>(
                    new ImpliedTermStructure(arguments_.riskFreeTS,
                                             arguments_.resetDate, 
                                             arguments_.resetDate)));

            // The following approach is plain wrong.
            // The right solution would be stochastic volatility or
            // at least local volatility
            // As a bare minimum one could extract from the Black vol surface
            // the implied vol at moneyness% of the forward value,
            // istead of the moneyness% of the spot value
            originalArguments_->volTS = 
                RelinkableHandle<BlackVolTermStructure>(
                    Handle<BlackVolTermStructure>(
                        new BlackConstantVol(
                            arguments_.resetDate,
                            arguments_.volTS->blackForwardVol(
                                arguments_.resetDate,
                                arguments_.exercise.date(),
                                arguments_.moneyness*arguments_.underlying),
                            arguments_.volTS->dayCounter())));

            originalArguments_->exercise = arguments_.exercise;

            originalArguments_->validate();
            originalEngine_->calculate();

            Time resetTime = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(), arguments_.resetDate);
            double discR = arguments_.riskFreeTS->discount(arguments_.resetDate);

            results_.value = discR * originalResults_->value;
            results_.delta = 0.0;
            results_.gamma = 0.0;
            results_.theta = arguments_.riskFreeTS->zeroYield(
                arguments_.resetDate) * results_.value;
            results_.vega = discR * originalResults_->vega;
            results_.rho = - resetTime * results_.value +
                discR * originalResults_->rho;
            results_.dividendRho = discR * originalResults_->dividendRho;

        }

    }

}

