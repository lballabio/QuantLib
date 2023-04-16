/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/exercise.hpp>
#include <ql/experimental/exoticoptions/analyticamericanmargrabeengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <utility>

namespace QuantLib {

    AnalyticAmericanMargrabeEngine::AnalyticAmericanMargrabeEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process1,
        std::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Real correlation)
    : process1_(std::move(process1)), process2_(std::move(process2)), rho_(correlation) {
        registerWith(process1_);
        registerWith(process2_);
    }

    void AnalyticAmericanMargrabeEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American option");

        std::shared_ptr<AmericanExercise> exercise =
            std::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an American option");

        std::shared_ptr<NullPayoff> payoff0 =
            std::dynamic_pointer_cast<NullPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff0, "not a null payoff");

        // The option can be priced as an American single-asset option
        // with an adjusted process and payoff.

        Date today = Settings::instance().evaluationDate();

        DayCounter rfdc  = process1_->riskFreeRate()->dayCounter();
        Time t = rfdc.yearFraction(process1_->riskFreeRate()->referenceDate(),
                                   arguments_.exercise->lastDate());

        Real s1 = process1_->stateVariable()->value();
        Real s2 = process2_->stateVariable()->value();

        std::shared_ptr<SimpleQuote> spot(new SimpleQuote(arguments_.Q1*s1));

        std::shared_ptr<StrikedTypePayoff> payoff(
                      new PlainVanillaPayoff(Option::Call, arguments_.Q2*s2));

        DiscountFactor dividendDiscount1 =
            process1_->dividendYield()->discount(exercise->lastDate());
        Rate q1 = -std::log(dividendDiscount1)/t;

        DiscountFactor dividendDiscount2 =
            process2_->dividendYield()->discount(exercise->lastDate());
        Rate q2 = -std::log(dividendDiscount2)/t;

        std::shared_ptr<YieldTermStructure> qTS(
                                            new FlatForward(today, q1, rfdc));

        std::shared_ptr<YieldTermStructure> rTS(
                                            new FlatForward(today, q2, rfdc));

        Real variance1 = process1_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), s1);
        Real variance2 = process2_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), s2);
        Real variance = variance1 + variance2
                      - 2*rho_*std::sqrt(variance1)*std::sqrt(variance2);
        Volatility volatility = std::sqrt(variance/t);

        std::shared_ptr<BlackVolTermStructure> volTS(
               new BlackConstantVol(today, NullCalendar(), volatility, rfdc));

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        std::shared_ptr<PricingEngine> engine(
                     new BjerksundStenslandApproximationEngine(stochProcess));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        results_.value = option.NPV();
    }

}
