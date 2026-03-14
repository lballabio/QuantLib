/*
 Copyright (C) 2021 Skandinaviska Enskilda Banken AB (publ)

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
#include <ql/pricingengines/asian/turnbullwakemanasianengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>

using namespace QuantLib;

void TurnbullWakemanAsianEngine::calculate() const {

    // Enforce a few required things
    QL_REQUIRE(arguments_.exercise->type() == Exercise::European, "not a European Option");
    QL_REQUIRE(arguments_.averageType == Average::Type::Arithmetic,
               "must be Arithmetic Average::Type");

    // Calculate the accrued portion
    Size pastFixings = arguments_.pastFixings;
    Size futureFixings = arguments_.fixingDates.size();
    Real accruedAverage = 0;
    if (pastFixings != 0) {
        accruedAverage = arguments_.runningAccumulator / (pastFixings + futureFixings);
    }
    results_.additionalResults["accrued"] = accruedAverage;

    Real discount = process_->riskFreeRate()->discount(arguments_.exercise->lastDate());
    results_.additionalResults["discount"] = discount;

    ext::shared_ptr<PlainVanillaPayoff> payoff =
        ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
    QL_REQUIRE(payoff, "non-plain payoff given");

    // We will read the volatility off the surface at the effective strike
    Real effectiveStrike = payoff->strike() - accruedAverage;
    results_.additionalResults["strike"] = payoff->strike();
    results_.additionalResults["effective_strike"] = effectiveStrike;

    // If the effective strike is negative, exercise resp. permanent OTM is guaranteed and the
    // valuation is made easy
    Size m = futureFixings + pastFixings;
    if (effectiveStrike <= 0.0) {
        // For a reference, see "Option Pricing Formulas", Haug, 2nd ed, p. 193
        if (payoff->optionType() == Option::Type::Call) {
            Real spot = process_->stateVariable()->value();
            Real S_A_hat = accruedAverage;
            for (const auto& fd : arguments_.fixingDates) {
                S_A_hat += (spot * process_->dividendYield()->discount(fd) /
                            process_->riskFreeRate()->discount(fd)) /
                           m;
            }
            results_.value = discount * (S_A_hat - payoff->strike());
            results_.delta = discount * (S_A_hat - accruedAverage) / spot;
        } else if (payoff->optionType() == Option::Type::Put) {
            results_.value = 0;
            results_.delta = 0;
        }
        results_.gamma = 0;
        return;
    }

    // We should only get this far when the effectiveStrike > 0 but will check anyway
    QL_REQUIRE(effectiveStrike > 0.0, "expected effectiveStrike to be positive");

    // Expected value of the non-accrued portion of the average prices
    // In general, m will equal n below if there is no accrued. If accrued, m > n.
    Real EA = 0.0;
    std::vector<Real> forwards;
    std::vector<Time> times;
    std::vector<Real> spotVars;
    std::vector<Real> spotVolsVec; // additional results only
    Real spot = process_->stateVariable()->value();

    for (const auto& fd : arguments_.fixingDates) {
        DiscountFactor dividendDiscount = process_->dividendYield()->discount(fd);
        DiscountFactor riskFreeDiscountForFwdEstimation = process_->riskFreeRate()->discount(fd);

        forwards.push_back(spot * dividendDiscount / riskFreeDiscountForFwdEstimation);
        times.push_back(process_->blackVolatility()->timeFromReference(fd));

        spotVars.push_back(
            process_->blackVolatility()->blackVariance(times.back(), effectiveStrike));
        spotVolsVec.push_back(std::sqrt(spotVars.back() / times.back()));

        EA += forwards.back();
    }
    EA /= m;

    // Expected value of A^2.
    Real EA2 = 0.0;
    Size n = forwards.size();

    for (Size i = 0; i < n; ++i) {
        EA2 += forwards[i] * forwards[i] * exp(spotVars[i]);
        for (Size j = 0; j < i; ++j) {
            EA2 += 2 * forwards[i] * forwards[j] * exp(spotVars[j]);
        }
    }

    EA2 /= m * m;

    // Calculate value
    Real tn = times.back();
    Real sigma = sqrt(log(EA2 / (EA * EA)) / tn);

    // Populate results
    BlackCalculator black(payoff->optionType(), effectiveStrike, EA, sigma * sqrt(tn), discount);

    results_.value = black.value();
    results_.delta = black.delta(spot);
    results_.gamma = black.gamma(spot);

    results_.additionalResults["forward"] = EA;
    results_.additionalResults["exp_A_2"] = EA2;
    results_.additionalResults["tte"] = tn;
    results_.additionalResults["sigma"] = sigma;
    results_.additionalResults["times"] = times;
    results_.additionalResults["spotVols"] = spotVolsVec;
    results_.additionalResults["forwards"] = forwards;
}
