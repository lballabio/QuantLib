/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    HestonModelHelper::HestonModelHelper(
                              const Period& maturity,
                              const Calendar& calendar,
                              const Real s0,
                              const Real strikePrice,
                              const Handle<Quote>& volatility,
                              const Handle<YieldTermStructure>& riskFreeRate,
                              const Handle<YieldTermStructure>& dividendYield,
                              bool calibrateVolatility)
    : CalibrationHelper(volatility, riskFreeRate, calibrateVolatility),
      dividendYield_(dividendYield),
      exerciseDate_(calendar.advance(riskFreeRate->referenceDate(),
                                     maturity)),
      tau_(riskFreeRate->dayCounter().yearFraction(
                               riskFreeRate->referenceDate(), exerciseDate_)),
      s0_(s0), strikePrice_(strikePrice) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                          new PlainVanillaPayoff(Option::Call, strikePrice_));

        boost::shared_ptr<Exercise> exercise(
                                         new EuropeanExercise(exerciseDate_));

        Handle<Quote> uly  (boost::shared_ptr<Quote>(new SimpleQuote(s0_)));
        boost::shared_ptr<StochasticProcess> dummyProcess(
                      new HestonProcess(riskFreeRate, dividendYield,
                                        uly, 1.0, 0.1, 1.0, 0.3, 0.0));

        option_ = boost::shared_ptr<VanillaOption>(
                           new VanillaOption(dummyProcess, payoff, exercise));

        marketValue_ = blackPrice(volatility->value());
    }

    Real HestonModelHelper::modelValue() const {
        option_->setPricingEngine(engine_);
        return option_->NPV();
    }

    Real HestonModelHelper::blackPrice(Real sigma) const {
        const Real volatility = sigma*std::sqrt(maturity());
        return blackFormula(Option::Call,
            strikePrice_*termStructure_->discount(tau_),
            s0_*dividendYield_->discount(tau_),
            volatility);
    }

}

