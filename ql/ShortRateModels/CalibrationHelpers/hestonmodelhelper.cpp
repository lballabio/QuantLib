/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/ShortRateModels/CalibrationHelpers/hestonmodelhelper.hpp>
#include <ql/Processes/hestonprocess.hpp>
#include <ql/Instruments/payoffs.hpp>

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
    : CalibrationHelper(volatility, riskFreeRate),
      dividendYield_(dividendYield),
      exerciseDate_(calendar.advance(riskFreeRate->referenceDate(),
                                     maturity)),
      tau_(riskFreeRate->dayCounter().yearFraction(
                               riskFreeRate->referenceDate(), exerciseDate_)),
      s0_(s0), strikePrice_(strikePrice),
      calibrateVolatility_(calibrateVolatility) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                          new PlainVanillaPayoff(Option::Call, strikePrice_));

        boost::shared_ptr<Exercise> exercise(
                                         new EuropeanExercise(exerciseDate_));

        Handle<Quote> underlying(boost::shared_ptr<Quote>(
                                                       new SimpleQuote(s0_)));
        boost::shared_ptr<StochasticProcess> process(
                      new HestonProcess(riskFreeRate, dividendYield,
                                        underlying, 1.0, 0.1, 1.0, 0.3, 0.0));

        option_ = boost::shared_ptr<VanillaOption>(
                                new VanillaOption(process, payoff, exercise));

        marketValue_ = blackPrice(volatility->value());
    }

    Real HestonModelHelper::modelValue() const {
        option_->setPricingEngine(engine_);
        return option_->NPV();
    }

    Real HestonModelHelper::blackPrice(Real sigma) const {
        const Real volatility = sigma*std::sqrt(maturity());
        return BlackModel::formula(s0_*dividendYield_->discount(tau_),
                                   strikePrice_*termStructure_->discount(tau_),
                                   volatility, 1.0);
    }

    Real HestonModelHelper::calibrationError() {
        if (calibrateVolatility_) {
            const Real lowerPrice = blackPrice(0.01);
            const Real upperPrice = blackPrice(5);
            const Real modelPrice = modelValue();
            
            Volatility implied;
            if (modelPrice <= lowerPrice)
                implied = 0.01;
            else
                if (modelPrice >= upperPrice)
                    implied = 5.0;
                else
                    implied = this->impliedVolatility(
                                        modelValue(), 1e-8, 5000, 0.01, 5);

            return implied - volatility_->value();            
        } else {
            return CalibrationHelper::calibrationError();
        }
    }

}

