/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file hestonblackvolsurface.hpp
    \brief Black volatility surface back by Heston model
*/

#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <limits>
#include <utility>

namespace QuantLib {

    namespace {
        Real blackValue(Option::Type optionType, Real strike,
                        Real forward, Real maturity,
                        Volatility vol, Real discount, Real npv) {

            return blackFormula(optionType, strike, forward,
                                std::max(0.0, vol)*std::sqrt(maturity),
                                discount)-npv;
        }
    }

    HestonBlackVolSurface::HestonBlackVolSurface(
        const Handle<HestonModel>& hestonModel,
        const AnalyticHestonEngine::ComplexLogFormula cpxLogFormula,
        AnalyticHestonEngine::Integration integration)
    : BlackVolTermStructure(hestonModel->process()->riskFreeRate()->referenceDate(),
                            NullCalendar(),
                            Following,
                            hestonModel->process()->riskFreeRate()->dayCounter()),
      hestonModel_(hestonModel), cpxLogFormula_(cpxLogFormula),
      integration_(std::move(integration)) {
        registerWith(hestonModel_);
    }

    DayCounter HestonBlackVolSurface::dayCounter() const {
        return hestonModel_->process()->riskFreeRate()->dayCounter();
    }
    Date HestonBlackVolSurface::maxDate() const {
        return Date::maxDate();
    }
    Real HestonBlackVolSurface::minStrike() const {
        return 0.0;
    }
    Real HestonBlackVolSurface::maxStrike() const {
        return std::numeric_limits<Real>::max();
    }

    Real HestonBlackVolSurface::blackVarianceImpl(Time t, Real strike) const {
        return squared(blackVolImpl(t, strike))*t;
    }

    Volatility HestonBlackVolSurface::blackVolImpl(Time t, Real strike) const {
        AnalyticHestonEngine hestonEngine(
                    hestonModel_.currentLink(), cpxLogFormula_, integration_);

        const ext::shared_ptr<HestonProcess>& process = hestonModel_->process();

        const DiscountFactor df = process->riskFreeRate()->discount(t, true);

        const Real fwd = process->s0()->value()
            * process->dividendYield()->discount(t, true) / df;

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(
                            fwd > strike ? Option::Put : Option::Call, strike);

        const Real npv = hestonEngine.priceVanillaPayoff(payoff, t);

        const Real theta = hestonModel_->theta();
        if (npv <= 0.0) return std::sqrt(theta);

        Brent solver;
        solver.setMaxEvaluations(10000);
        const Volatility guess = std::sqrt(theta);
        constexpr double accuracy = std::numeric_limits<double>::epsilon();

        return solver.solve(
            [&](Volatility _v) {
                return blackValue(
                    payoff->optionType(), strike, fwd, t, _v, df, npv);
            },
            accuracy, guess, 0.01
        );
    }
}
