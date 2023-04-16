/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 StatPro Italia srl

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

#include <ql/instruments/impliedvolatility.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    namespace {

        class PriceError {
          public:
            PriceError(const PricingEngine& engine,
                       SimpleQuote& vol,
                       Real targetValue);
            Real operator()(Volatility x) const;
          private:
            const PricingEngine& engine_;
            SimpleQuote& vol_;
            Real targetValue_;
            const Instrument::results* results_;
        };

        PriceError::PriceError(const PricingEngine& engine,
                               SimpleQuote& vol,
                               Real targetValue)
        : engine_(engine), vol_(vol), targetValue_(targetValue) {
            results_ =
                dynamic_cast<const Instrument::results*>(engine_.getResults());
            QL_REQUIRE(results_ != nullptr, "pricing engine does not supply needed results");
        }

        Real PriceError::operator()(Volatility x) const {
            vol_.setValue(x);
            engine_.calculate();
            return results_->value-targetValue_;
        }

    }


    namespace detail {

        Volatility ImpliedVolatilityHelper::calculate(
                                                 const Instrument& instrument,
                                                 const PricingEngine& engine,
                                                 SimpleQuote& volQuote,
                                                 Real targetValue,
                                                 Real accuracy,
                                                 Natural maxEvaluations,
                                                 Volatility minVol,
                                                 Volatility maxVol) {

            instrument.setupArguments(engine.getArguments());
            engine.getArguments()->validate();

            PriceError f(engine, volQuote, targetValue);
            Brent solver;
            solver.setMaxEvaluations(maxEvaluations);
            Volatility guess = (minVol+maxVol)/2.0;
            Volatility result = solver.solve(f, accuracy, guess,
                                             minVol, maxVol);
            return result;
        }

        std::shared_ptr<GeneralizedBlackScholesProcess>
        ImpliedVolatilityHelper::clone(
             const std::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const std::shared_ptr<SimpleQuote>& volQuote) {

            Handle<Quote> stateVariable = process->stateVariable();
            Handle<YieldTermStructure> dividendYield = process->dividendYield();
            Handle<YieldTermStructure> riskFreeRate = process->riskFreeRate();

            Handle<BlackVolTermStructure> blackVol = process->blackVolatility();
            Handle<BlackVolTermStructure> volatility(
                std::shared_ptr<BlackVolTermStructure>(
                               new BlackConstantVol(blackVol->referenceDate(),
                                                    blackVol->calendar(),
                                                    Handle<Quote>(volQuote),
                                                    blackVol->dayCounter())));

            return std::make_shared<GeneralizedBlackScholesProcess>(
                stateVariable, dividendYield,
                                                   riskFreeRate, volatility);
        }

    }

}
