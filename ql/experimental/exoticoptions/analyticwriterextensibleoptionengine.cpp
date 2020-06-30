/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/experimental/exoticoptions/analyticwriterextensibleoptionengine.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/pricingengines/blackformula.hpp>

using namespace std;

namespace QuantLib {

    AnalyticWriterExtensibleOptionEngine::AnalyticWriterExtensibleOptionEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void AnalyticWriterExtensibleOptionEngine::calculate() const {
        // We take all the arguments:

        ext::shared_ptr<PlainVanillaPayoff> payoff1 =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff1, "not a plain vanilla payoff");

        ext::shared_ptr<PlainVanillaPayoff> payoff2 =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff2);
        QL_REQUIRE(payoff2, "not a plain vanilla payoff");

        ext::shared_ptr<Exercise> exercise1 = arguments_.exercise;

        ext::shared_ptr<Exercise> exercise2 = arguments_.exercise2;


        // We create and apply the calculate process:

        Option::Type type = payoff1->optionType();

        // STEP 1:

        // S = spot
        Real spot = process_->stateVariable()->value();

        // For the B&S formulae:
        DayCounter dividendDC = process_->dividendYield()->dayCounter();
        Rate dividend = process_->dividendYield()->zeroRate(
                  exercise1->lastDate(), dividendDC, Continuous, NoFrequency);

        DayCounter riskFreeDC = process_->riskFreeRate()->dayCounter();
        Rate riskFree = process_->riskFreeRate()->zeroRate(
                  exercise1->lastDate(), riskFreeDC, Continuous, NoFrequency);

        // The time to maturity:
        Time t1 = riskFreeDC.yearFraction(
                                    process_->riskFreeRate()->referenceDate(),
                                    arguments_.exercise->lastDate());
        Time t2 = riskFreeDC.yearFraction(
                                    process_->riskFreeRate()->referenceDate(),
                                    arguments_.exercise2->lastDate());

        // b = r-q:
        Real b = riskFree - dividend;

        Real forwardPrice = spot * std::exp(b*t1);

        Volatility volatility = process_->blackVolatility()->blackVol(
                                    exercise1->lastDate(), payoff1->strike());

        Real stdDev = volatility*std::sqrt(t1);

        Real discount = std::exp(-riskFree*t1);

        // Call the B&S method:
        Real black = blackFormula(type, payoff1->strike(),
                                  forwardPrice, stdDev, discount);

        // STEP 2:

        // Standard bivariate normal distribution:
        Real ro = std::sqrt(t1/t2);
        Real z1 = (std::log(spot/payoff2->strike()) +
                   (b+std::pow(volatility, 2)/2)*t2)/(volatility*std::sqrt(t2));
        Real z2 = (std::log(spot/payoff1->strike()) +
                   (b+std::pow(volatility, 2)/2)*t1)/(volatility*std::sqrt(t1));

        // Call the bivariate method:
        BivariateCumulativeNormalDistributionWe04DP biv(-ro);


        // STEP 3:

        Real bivariate1, bivariate2, result;

        // Final computing:
        if (type == Option::Call) {
            // Call case:
            bivariate1 = biv(z1, -z2);
            bivariate2 = biv(z1-volatility*std::sqrt(t2),
                             -z2+volatility*std::sqrt(t1));
            result = black + spot*std::exp((b-riskFree)*t2)*bivariate1
                - payoff2->strike()*std::exp((-riskFree)*t2)*bivariate2;
        } else {
            // Put case:
            bivariate1 = biv(-z1, z2);
            bivariate2 = biv(-z1+volatility*std::sqrt(t2),
                             z2-volatility*std::sqrt(t1));
            result = black - spot*std::exp((b-riskFree)*t2)*bivariate1
                + payoff2->strike()*std::exp((-riskFree)*t2)*bivariate2;
        }

        // Save the result:
        results_.value = result;
    }

}
