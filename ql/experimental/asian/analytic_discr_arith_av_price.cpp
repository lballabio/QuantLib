/*
 Copyright (C) 2021 Skandinaviska Enskilda Banken AB (publ)

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

#include <ql/experimental/asian/analytic_discr_arith_av_price.hpp>

#include <ql/pricingengines/blackformula.hpp>
#include <ql/exercise.hpp>

using namespace QuantLib;

void AnalyticDiscreteArithmeticAveragePriceAsianEngine::calculate() const {

    // Enforce a few required things
    QL_REQUIRE(arguments_.exercise->type() == Exercise::European, "not a European Option");
    QL_REQUIRE(arguments_.averageType == Average::Type::Arithmetic,
               "must be Arithmetic Average::Type");

    // Calculate the accrued portion
    Real runningAccumulator = arguments_.runningAccumulator;
    Size pastFixings = arguments_.pastFixings;
    Real accruedAverage = 0;
    if (pastFixings != 0) {
        accruedAverage = runningAccumulator / (pastFixings + arguments_.fixingDates.size());
    }

    // Populate some additional results that don't change
    auto& addRes = results_.additionalResults;
    Real discount = process_->riskFreeRate()->discount(arguments_.exercise->lastDate());
    addRes["discount"] = discount;
    addRes["accrued"] = accruedAverage;

    ext::shared_ptr<PlainVanillaPayoff> payoff =
        ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
    QL_REQUIRE(payoff, "non-plain payoff given");

    // TODO: If not model dependent, return early.
    Size m = arguments_.fixingDates.size() + pastFixings;
    /*if (pastFixings > 0) {

        if (accruedAverage > 1.0 * arguments_.fixingDates.size() / pastFixings * payoff->strike()) {
            if (payoff->optionType() == Option::Type::Call) {
                results_.value = 1010101;
            } else if (payoff->optionType() == Option::Type::Put) {
                results_.value = 0;
                return;
            } else {
                QL_FAIL("unexpected option type " << payoff->optionType());
            }
        }
    }*/

    // We will read the volatility off the surface at the effective strike
    // We should only get this far when the effectiveStrike > 0 but will check anyway
    Real effectiveStrike = payoff->strike() - accruedAverage;
    QL_REQUIRE(effectiveStrike > 0.0, "expected effectiveStrike to be positive");

    // Valuation date
    Date today = Settings::instance().evaluationDate();

    // Expected value of the non-accrued portion of the average prices
    // In general, m will equal n below if there is no accrued. If accrued, m > n.
    Real EA = 0.0;
    std::vector<Real> forwards;
    std::vector<Time> times;
    std::vector<Real> spotVars;
    std::vector<Real> spotVolsVec; // additional results only
    for (const auto& fd : arguments_.fixingDates) {
        if (fd >= today) {
            Real spot = process_->stateVariable()->value();
            DiscountFactor dividendDiscount = process_->dividendYield()->discount(fd);
            DiscountFactor riskFreeDiscountForFwdEstimation =
                process_->riskFreeRate()->discount(fd);

            forwards.push_back(spot * dividendDiscount / riskFreeDiscountForFwdEstimation);
            times.push_back(process_->blackVolatility()->timeFromReference(fd));

            spotVars.push_back(
                process_->blackVolatility()->blackVariance(times.back(), effectiveStrike));
            spotVolsVec.push_back(std::sqrt(spotVars.back() / times.back()));

            EA += forwards.back();
        }
    }
    EA /= m;

    // Expected value of A^2.
    Real EA2 = 0.0;
    Size n = forwards.size();

    // References spot prices
    for (Size i = 0; i < n; ++i) {
        EA2 += forwards[i] * forwards[i] * exp(spotVars[i]);
        for (Size j = 0; j < i; ++j) {
            EA2 += 2 * forwards[i] * forwards[j] * exp(spotVars[j]);
        }
    }
    addRes["spotVols"] = spotVolsVec;

    EA2 /= m * m;

    // Calculate value
    Real tn = times.back();
    Real sigma = sqrt(log(EA2 / (EA * EA)) / tn);

    // Populate results
    results_.value =
        blackFormula(payoff->optionType(), effectiveStrike, EA, sigma * sqrt(tn), discount);

    // Add more additional results
    addRes["strike"] = payoff->strike();
    addRes["effective_strike"] = effectiveStrike;
    addRes["forward"] = EA;
    addRes["exp_A_2"] = EA2;
    addRes["tte"] = tn;
    addRes["sigma"] = sigma;
    addRes["times"] = times;
    addRes["forwards"] = forwards;
}
