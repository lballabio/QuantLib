/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers

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
                            CalibrationHelper::CalibrationErrorType errorType)
    : CalibrationHelper(volatility, riskFreeRate, errorType),
      maturity_(maturity), calendar_(calendar),
      s0_(Handle<Quote>(ext::make_shared<SimpleQuote>(s0))),
      strikePrice_(strikePrice), dividendYield_(dividendYield) {
        registerWith(dividendYield);
    }

    HestonModelHelper::HestonModelHelper(
                            const Period& maturity,
                            const Calendar& calendar,
                            const Handle<Quote>& s0,
                            const Real strikePrice,
                            const Handle<Quote>& volatility,
                            const Handle<YieldTermStructure>& riskFreeRate,
                            const Handle<YieldTermStructure>& dividendYield,
                            CalibrationHelper::CalibrationErrorType errorType)
    : CalibrationHelper(volatility, riskFreeRate, errorType),
      maturity_(maturity), calendar_(calendar), s0_(s0),
      strikePrice_(strikePrice), dividendYield_(dividendYield) {
        registerWith(s0);
        registerWith(dividendYield);
    }

    void HestonModelHelper::performCalculations() const {
        exerciseDate_ =
            calendar_.advance(termStructure_->referenceDate(), maturity_);
        tau_ = termStructure_->timeFromReference(exerciseDate_);
        type_ = strikePrice_ * termStructure_->discount(tau_) >=
                        s0_->value() * dividendYield_->discount(tau_)
                    ? Option::Call
                    : Option::Put;
        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(type_, strikePrice_));
        ext::shared_ptr<Exercise> exercise =
            ext::make_shared<EuropeanExercise>(exerciseDate_);
        option_ = ext::make_shared<VanillaOption>(payoff, exercise);
        CalibrationHelper::performCalculations();
    }

    Real HestonModelHelper::modelValue() const {
        calculate();
        option_->setPricingEngine(engine_);
        return option_->NPV();
    }

    Real HestonModelHelper::blackPrice(Real volatility) const {
        calculate();
        const Real stdDev = volatility * std::sqrt(maturity());
        return blackFormula(
            type_, strikePrice_ * termStructure_->discount(tau_),
            s0_->value() * dividendYield_->discount(tau_), stdDev);
    }
}

