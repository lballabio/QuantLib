/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014, 2015 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file analyticpdfhestonengine.cpp
    \brief Analytic engine for arbitrary European payoffs under the Heston model
*/

#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/methods/finitedifferences/utilities/hestonrndcalculator.hpp>
#include <utility>

namespace QuantLib {

    AnalyticPDFHestonEngine::AnalyticPDFHestonEngine(std::shared_ptr<HestonModel> model,
                                                     Real integrationEps_,
                                                     Size maxIntegrationIterations)
    : maxIntegrationIterations_(maxIntegrationIterations), integrationEps_(integrationEps_),
      model_(std::move(model)) {}

    void AnalyticPDFHestonEngine::calculate() const {
        // this is an European option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        const std::shared_ptr<HestonProcess>& process = model_->process();

        const Time t = process->time(arguments_.exercise->lastDate());

        const Real xMax = 8.0 * std::sqrt(process->theta()*t
            + (process->v0() - process->theta())
                *(1-std::exp(-process->kappa()*t))/process->kappa());

        const Real x0 = std::log(process->s0()->value());
        const Real rD = process->riskFreeRate()->discount(t);
        const Real qD = process->dividendYield()->discount(t);

        const Real drift = x0 + std::log(rD/qD);

        results_.value = GaussLobattoIntegral(maxIntegrationIterations_, integrationEps_)(
            [&](Real _x){ return weightedPayoff(_x, t); },
            -xMax+drift, xMax+drift);
    }

    Real AnalyticPDFHestonEngine::Pv(Real x_t, Time t) const {
        return HestonRNDCalculator(
            model_->process(), integrationEps_, maxIntegrationIterations_)
                .pdf(x_t, t);
    }

    Real AnalyticPDFHestonEngine::cdf(Real s, Time t) const {
        const Real x_t = std::log(s);
        return HestonRNDCalculator(
            model_->process(), integrationEps_, maxIntegrationIterations_)
                .cdf(x_t, t);
    }

    Real AnalyticPDFHestonEngine::weightedPayoff(Real x_t, Time t) const {
        const DiscountFactor rD
            = model_->process()->riskFreeRate()->discount(t);

        const Real s_t = std::exp(x_t);
        const Real payoff = (*arguments_.payoff)(s_t);

        return (payoff != 0.0) ? payoff*Pv(x_t, t)*rD : Real(0.0);
    }
}

