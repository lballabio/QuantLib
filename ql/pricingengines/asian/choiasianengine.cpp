/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Klaus Spanderen

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
#include <ql/quotes/simplequote.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/asian/choiasianengine.hpp>
#include <ql/pricingengines/basket/choibasketengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>

namespace QuantLib {

    ChoiAsianEngine::ChoiAsianEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Real lambda,
        Size maxNrIntegrationSteps)
    : process_(std::move(process)),
      lambda_(lambda),
      maxNrIntegrationSteps_(maxNrIntegrationSteps) {
        registerWith(process_);
    }

    void ChoiAsianEngine::calculate() const {
        QL_REQUIRE(arguments_.averageType == Average::Type::Arithmetic,
            "must be Average::Type Arithmetic ");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
            "not a European Option");

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        std::vector<Date> fixingDates = arguments_.fixingDates;
        std::sort(fixingDates.begin(), fixingDates.end());

        Size futureFixings = fixingDates.size();
        Size pastFixings = arguments_.pastFixings;
        Real runningAccumulator = arguments_.runningAccumulator;

        const Date exerciseDate = arguments_.exercise->lastDate();
        const Handle<YieldTermStructure> rTS = process_->riskFreeRate();

        if (   futureFixings > 0
            && process_->time(fixingDates.front()) == Time(0)) {
            // push today fixing to past fixings
            fixingDates.erase(fixingDates.begin());
            futureFixings--;
            pastFixings++;
            runningAccumulator += process_->x0();
        }

        if (futureFixings == 0) {
            QL_REQUIRE(pastFixings > 0, "no past fixings given");
            results_.value = (*payoff)(runningAccumulator/pastFixings)
                * rTS->discount(exerciseDate);

            return;
        }

        QL_REQUIRE(fixingDates.back() <= exerciseDate,
            "last fixing date must be before exercise date");
        QL_REQUIRE(process_->time(fixingDates.front()) >= 0.0,
            "first fixing date is in the past");

        QL_REQUIRE(std::adjacent_find(fixingDates.begin(), fixingDates.end())
            == fixingDates.end(), "two fixing dates are the same");

        const Real accruedAverage = (pastFixings != 0)
            ? Real(runningAccumulator / (pastFixings + futureFixings))
            : 0.0;

        const Real strike = payoff->strike() - accruedAverage;
        QL_REQUIRE(strike >= 0.0, "effective strike should to be positive");

        const Handle<YieldTermStructure> qTS = process_->dividendYield();
        const Handle<BlackVolTermStructure> volTS = process_->blackVolatility();
        const Date volRefDate = volTS->referenceDate();
        const DayCounter volDc = volTS->dayCounter();

        if (futureFixings > 1) {
            std::vector<Time> fixingTimes(futureFixings), variances(futureFixings);
            for (Size i=0; i < futureFixings; ++i) {
                const Date& fixingDate = fixingDates[i];
                fixingTimes[i] = volDc.yearFraction(volRefDate,  fixingDate);
                variances[i] = process_->blackVolatility()->blackVariance(fixingDate, strike);
            }

            Matrix rho(futureFixings, futureFixings);
            for (Size i=0; i < rho.rows(); ++i)
                for (Size j=i; j < rho.columns(); ++j)
                    rho[i][j] = rho[j][i] =
                        variances[std::min(i,j)] / std::sqrt(variances[i]*variances[j]);

            const Handle<YieldTermStructure> zeroTS(
                ext::make_shared<FlatForward>(rTS->referenceDate(), 0.0, rTS->dayCounter())
            );

            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
            processes.reserve(futureFixings);
            for (Size i=0; i < futureFixings; ++i) {
                const Date& fixingDate = fixingDates[i];
                const Volatility sig = volTS->blackVol(fixingDate, payoff->strike())
                    * std::sqrt(fixingTimes[i]/fixingTimes.back());

                processes.emplace_back(
                    ext::make_shared<GeneralizedBlackScholesProcess>(
                        Handle<Quote>(
                            ext::make_shared<SimpleQuote>(
                                process_->x0()*qTS->discount(fixingDate)/rTS->discount(fixingDate)
                            )
                        ),
                        zeroTS, zeroTS,
                        Handle<BlackVolTermStructure>(
                           ext::make_shared<BlackConstantVol>(
                              volRefDate, volTS->calendar(),
                              Handle<Quote>(ext::make_shared<SimpleQuote>(sig)),
                              volDc
                           )
                        )
                    )
                );
            }

            BasketOption basketOption(
                ext::make_shared<AverageBasketPayoff>(
                    ext::make_shared<PlainVanillaPayoff>(payoff->optionType(), strike),
                    Array(futureFixings, 1.0/(futureFixings + pastFixings))
                ),
                ext::make_shared<EuropeanExercise>(fixingDates.back())
            );
            basketOption.setPricingEngine(
                ext::make_shared<ChoiBasketEngine>(
                    processes, rho, lambda_, maxNrIntegrationSteps_)
            );

            results_.value = basketOption.NPV() * rTS->discount(exerciseDate);
        }
        else if (futureFixings == 1) {
            results_.value = blackFormula(
                payoff->optionType(),
                strike,
                process_->x0()/(pastFixings + futureFixings)
                    *qTS->discount(fixingDates.back())/rTS->discount(fixingDates.back()),
                std::sqrt(volTS->blackVariance(fixingDates.back(), strike)),
                rTS->discount(exerciseDate)
            );
        }
    }
}
