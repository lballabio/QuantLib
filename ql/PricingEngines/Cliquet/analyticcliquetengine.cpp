
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Cliquet/analyticcliquetengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    namespace {

        Time time(const boost::shared_ptr<TermStructure>& ts,
                  const Date& d) {
            return ts->dayCounter().yearFraction(ts->referenceDate(), d);
        }

        Time time(const boost::shared_ptr<BlackVolTermStructure>& ts,
                  const Date& d) {
            return ts->dayCounter().yearFraction(ts->referenceDate(), d);
        }

    }

    void AnalyticCliquetEngine::calculate() const {

        QL_REQUIRE(arguments_.accruedCoupon == Null<double>() &&
                   arguments_.lastFixing == Null<double>(),
                   "this engine cannot price options already started");
        QL_REQUIRE(arguments_.localCap == Null<double>() &&
                   arguments_.localFloor == Null<double>() &&
                   arguments_.globalCap == Null<double>() &&
                   arguments_.globalFloor == Null<double>(),
                   "this engine cannot price capped/floored options");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        boost::shared_ptr<PercentageStrikePayoff> moneyness =
            boost::dynamic_pointer_cast<PercentageStrikePayoff>(
                                                           arguments_.payoff);
        QL_REQUIRE(moneyness, "wrong payoff given");

        const boost::shared_ptr<BlackScholesProcess>& process =
            arguments_.blackScholesProcess;

        std::vector<Date> resetDates = arguments_.resetDates;
        resetDates.push_back(arguments_.exercise->lastDate());

        double underlying = process->stateVariable()->value();
        double strike = underlying * moneyness->strike();
        boost::shared_ptr<StrikedTypePayoff> payoff(
                      new PlainVanillaPayoff(moneyness->optionType(),strike));

        results_.value = 0.0;
        results_.delta = results_.gamma = 0.0;
        results_.theta = 0.0;
        results_.rho = results_.dividendRho = 0.0;
        results_.vega = 0.0;

        for (Size i = 1; i < resetDates.size(); i++) {

            double weight = 
                process->dividendYield()->discount(resetDates[i-1]);
            DiscountFactor discount = 
                process->riskFreeRate()->discount(resetDates[i]) /
                process->riskFreeRate()->discount(resetDates[i-1]);
            DiscountFactor qDiscount = 
                process->dividendYield()->discount(resetDates[i]) /
                process->dividendYield()->discount(resetDates[i-1]);
            double forward = underlying*qDiscount/discount;
            double variance = 
                process->blackVolatility()->blackForwardVariance(
                                        resetDates[i-1],resetDates[i],strike);

            BlackFormula black(forward, discount, variance, payoff);

            results_.value += weight * black.value();
            results_.delta += weight * (black.delta(underlying) +
                                        moneyness->strike() * discount * 
                                        black.beta());
            results_.gamma += 0.0;
            results_.theta += process->dividendYield()
                ->forward(resetDates[i-1],resetDates[i]) * 
                weight * black.value();

            Time dt = process->riskFreeRate()->dayCounter().yearFraction(
                                               resetDates[i-1],resetDates[i]);
            results_.rho += weight * black.rho(dt);

            Time t = process->dividendYield()->dayCounter().yearFraction(
                                    process->dividendYield()->referenceDate(),
                                    resetDates[i-1]);
            dt = process->dividendYield()->dayCounter().yearFraction(
                                               resetDates[i-1],resetDates[i]);
            results_.dividendRho += weight * (black.dividendRho(dt) -
                                              t * black.value());

            dt = process->blackVolatility()->dayCounter().yearFraction(
                                               resetDates[i-1],resetDates[i]);
            results_.vega += weight * black.vega(dt);
        }

    }

}

