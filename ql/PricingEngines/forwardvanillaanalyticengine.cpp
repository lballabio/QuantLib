
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
/*! \file forwardvanillaanalyticengine.cpp
    \brief Forward vanilla engine using analytic formulas

    \fullpath
    ql/Pricers/%forwardvanillaanalyticengine.cpp
*/

// $Id$

#include <ql/PricingEngines/forwardengines.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>

using QuantLib::TermStructures::ImpliedTermStructure;
using QuantLib::VolTermStructures::BlackConstantVol;

namespace QuantLib {

    namespace PricingEngines {

        void ForwardVanillaAnalyticEngine::calculate() const {

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = 1.0;
            originalArguments_->strike = arguments_.moneyness;
            originalArguments_->dividendTS = Handle<TermStructure>(new
                ImpliedTermStructure(arguments_.dividendTS,
                arguments_.resetDate, arguments_.resetDate));
            originalArguments_->riskFreeTS = Handle<TermStructure>(new
                ImpliedTermStructure(arguments_.riskFreeTS,
                arguments_.resetDate, arguments_.resetDate));

            // The following approach is plain wrong.
            // The right solution would be stochastic volatility or
            // at least local volatility
            // As a bare minimum one could extract from the Black vol surface
            // the implied vol at moneyness% of the forward value,
            // istead of the moneyness% of the spot value
            originalArguments_->volTS = Handle<BlackVolTermStructure>(new
                BlackConstantVol(arguments_.resetDate,
                arguments_.volTS->blackForwardVol(arguments_.resetDate,
                arguments_.exercise.date(),
                arguments_.moneyness* arguments_.underlying)));

            originalArguments_->exercise = arguments_.exercise;


            originalArguments_->validate();
            originalEngine_->calculate();

            Time resetTime = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(), arguments_.resetDate);
            double discQ = arguments_.dividendTS->discount(arguments_.resetDate);

            results_.value = discQ * arguments_.underlying *
                originalResults_->value;
            results_.delta = discQ * originalResults_->value;
            results_.gamma = 0.0;
            results_.theta = arguments_.dividendTS->zeroYield(arguments_.resetDate) *
                results_.value;
            results_.vega = discQ * arguments_.underlying *
                originalResults_->vega;
            results_.rho = discQ * arguments_.underlying *
                originalResults_->rho;
            results_.dividendRho = - resetTime * results_.value
                + discQ * originalResults_->dividendRho;

        }

    }

}

