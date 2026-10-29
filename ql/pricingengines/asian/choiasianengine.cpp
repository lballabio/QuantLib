/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Klaus Spanderen

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

        // Effective strike non-positive: the average is bounded below by the
        // accrued average (A = accruedAverage + sum_i F_i/m with F_i >= 0),
        // so A >= payoff->strike() a.s. and exercise is guaranteed. See
        // "Option Pricing Formulas", Haug, 2nd ed, p.193. This mirrors
        // TurnbullWakemanAsianEngine.
        if (strike <= 0.0) {
            Real expectedAverage = accruedAverage;
            for (const auto& fd : fixingDates) {
                expectedAverage += process_->x0()
                    * process_->dividendYield()->discount(fd)
                    / process_->riskFreeRate()->discount(fd)
                    / (pastFixings + futureFixings);
            }

            if (payoff->optionType() == Option::Type::Call) {
                results_.value = rTS->discount(exerciseDate)
                    * (expectedAverage - payoff->strike());
                results_.delta = rTS->discount(exerciseDate)
                    * (expectedAverage - accruedAverage) / process_->x0();
            } else if (payoff->optionType() == Option::Type::Put) {
                results_.value = 0.0;
                results_.delta = 0.0;
            }
            results_.gamma = 0.0;
            return;
        }

        QL_REQUIRE(strike > 0.0, "effective strike should be positive");

        const Handle<YieldTermStructure> qTS = process_->dividendYield();
        const Handle<BlackVolTermStructure> volTS = process_->blackVolatility();
        const Date volRefDate = volTS->referenceDate();
        const DayCounter volDc = volTS->dayCounter();

        // Split the fixings into zero-variance (deterministic) and stochastic
        // ones. A zero variance would make the pairwise correlation below 0/0;
        // dropping the deterministic fixings from the basket and folding their
        // forwards into the strike is the exact continuous extension.
        Real deterministicForward = 0.0;
        std::vector<Date> stochasticDates;
        std::vector<Time> fixingTimes;
        std::vector<Real> variances;
        for (Size i=0; i < futureFixings; ++i) {
            const Date& fixingDate = fixingDates[i];
            const Real variance = volTS->blackVariance(fixingDate, strike);
            if (variance > 0.0) {
                stochasticDates.push_back(fixingDate);
                fixingTimes.push_back(volDc.yearFraction(volRefDate, fixingDate));
                variances.push_back(variance);
            } else {
                deterministicForward += process_->x0()
                    * qTS->discount(fixingDate) / rTS->discount(fixingDate)
                    / (pastFixings + futureFixings);
            }
        }

        if (variances.empty()) {
            // all remaining fixings are deterministic: closed form
            Real expectedAverage = accruedAverage + deterministicForward;
            Real expectedForwards = expectedAverage - accruedAverage;
            results_.value = rTS->discount(exerciseDate)
                * (*payoff)(expectedAverage);
            if (payoff->optionType() == Option::Type::Call) {
                results_.delta = expectedAverage > payoff->strike()
                    ? rTS->discount(exerciseDate) * expectedForwards / process_->x0()
                    : 0.0;
            } else {
                results_.delta = expectedAverage < payoff->strike()
                    ? -rTS->discount(exerciseDate) * expectedForwards / process_->x0()
                    : 0.0;
            }
            results_.gamma = 0.0;
            return;
        }

        const Real residualStrike = strike - deterministicForward;

        if (residualStrike <= 0.0) {
            // the stochastic part alone guarantees exercise, as above
            Real expectedAverage = accruedAverage + deterministicForward;
            for (const auto& fd : stochasticDates) {
                expectedAverage += process_->x0()
                    * qTS->discount(fd) / rTS->discount(fd)
                    / (pastFixings + futureFixings);
            }

            if (payoff->optionType() == Option::Type::Call) {
                results_.value = rTS->discount(exerciseDate)
                    * (expectedAverage - payoff->strike());
                results_.delta = rTS->discount(exerciseDate)
                    * (expectedAverage - accruedAverage - deterministicForward)
                    / process_->x0();
            } else if (payoff->optionType() == Option::Type::Put) {
                results_.value = 0.0;
                results_.delta = 0.0;
            }
            results_.gamma = 0.0;
            return;
        }

        if (variances.size() == 1) {
            results_.value = blackFormula(
                payoff->optionType(),
                residualStrike,
                process_->x0()/(pastFixings + futureFixings)
                    *qTS->discount(stochasticDates.back())/rTS->discount(stochasticDates.back()),
                std::sqrt(variances.back()),
                rTS->discount(exerciseDate)
            );
        } else {
            Matrix rho(variances.size(), variances.size());
            for (Size i=0; i < rho.rows(); ++i)
                for (Size j=i; j < rho.columns(); ++j)
                    rho[i][j] = rho[j][i] =
                        variances[std::min(i,j)] / std::sqrt(variances[i]*variances[j]);

            const Handle<YieldTermStructure> zeroTS(
                ext::make_shared<FlatForward>(rTS->referenceDate(), 0.0, rTS->dayCounter())
            );

            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
            processes.reserve(variances.size());
            for (Size i=0; i < variances.size(); ++i) {
                const Date& fixingDate = stochasticDates[i];
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
                    ext::make_shared<PlainVanillaPayoff>(payoff->optionType(), residualStrike),
                    Array(variances.size(), 1.0/(futureFixings + pastFixings))
                ),
                ext::make_shared<EuropeanExercise>(stochasticDates.back())
            );
            basketOption.setPricingEngine(
                ext::make_shared<ChoiBasketEngine>(
                    processes, rho, lambda_, maxNrIntegrationSteps_)
            );

            results_.value = basketOption.NPV() * rTS->discount(exerciseDate);
        }
    }
}
