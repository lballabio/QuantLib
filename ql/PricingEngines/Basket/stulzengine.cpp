
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Neil Firth

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

#include <ql/PricingEngines/Basket/stulzengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Math/bivariatenormaldistribution.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace {

        // calculate the value of euro min basket call
        Real euroTwoAssetMinBasketCall(Real forward1, Real forward2,
                                       Real strike,
                                       DiscountFactor riskFreeDiscount,
                                       Real variance1, Real variance2,
                                       Real rho) {

            Real stdDev1 = std::sqrt(variance1);
            Real stdDev2 = std::sqrt(variance2);

            Real variance = variance1 + variance2 - 2*rho*stdDev1*stdDev2;
            Real stdDev = std::sqrt(variance);

            Real modRho1 = (rho * stdDev2 - stdDev1) / stdDev;
            Real modRho2 = (rho * stdDev1 - stdDev2) / stdDev;

            Real D1 = (std::log(forward1/forward2) + 0.5*variance) / stdDev;

            Real alfa, beta, gamma;
            if (strike != 0.0) {
                BivariateCumulativeNormalDistribution bivCNorm =
                    BivariateCumulativeNormalDistribution(rho);
                BivariateCumulativeNormalDistribution bivCNormMod2 =
                    BivariateCumulativeNormalDistribution(modRho2);
                BivariateCumulativeNormalDistribution bivCNormMod1 =
                    BivariateCumulativeNormalDistribution(modRho1);

                Real D1_1 =
                    (std::log(forward1/strike) + 0.5*variance1) / stdDev1;
                Real D1_2 =
                    (std::log(forward2/strike) + 0.5*variance2) / stdDev2;
                alfa = bivCNormMod1(D1_1, -D1);
                beta = bivCNormMod2(D1_2, D1 - stdDev);
                gamma = bivCNorm(D1_1 - stdDev1, D1_2 - stdDev2);
            } else {
                CumulativeNormalDistribution cum;
                alfa = cum(-D1);
                beta = cum(D1 - stdDev);
                gamma = 1.0;
            }

            return riskFreeDiscount *
                (forward1*alfa + forward2*beta - strike*gamma);

        }

        // calculate the value of euro max basket call
        Real euroTwoAssetMaxBasketCall(Real forward1, Real forward2,
                                       Real strike,
                                       DiscountFactor riskFreeDiscount,
                                       Real variance1, Real variance2,
                                       Real rho) {

            boost::shared_ptr<StrikedTypePayoff> payoff(new
                PlainVanillaPayoff(Option::Call, strike));

            BlackFormula black1(forward1, riskFreeDiscount,
                                variance1, payoff);
            BlackFormula black2(forward2, riskFreeDiscount,
                                variance2, payoff);

            return black1.value() + black2.value() -
                euroTwoAssetMinBasketCall(forward1, forward2, strike,
                                          riskFreeDiscount,
                                          variance1, variance2, rho);
        }
    }

    void StulzEngine::calculate() const {

        std::vector<boost::shared_ptr<BlackScholesProcess> > procs =
            arguments_.blackScholesProcesses;

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        QL_REQUIRE(procs.size() == 2,
                   "not a basket of two stocks");

        boost::shared_ptr<EuropeanExercise> exercise =
            boost::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European Option");

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real strike = payoff->strike();

        Real variance1 = procs[0]->blackVolatility()->blackVariance(
                                                exercise->lastDate(), strike);
        Real variance2 = procs[1]->blackVolatility()->blackVariance(
                                                exercise->lastDate(), strike);

        Real rho = arguments_.correlation[1][0];

        DiscountFactor riskFreeDiscount =
            procs[0]->riskFreeRate()->discount(exercise->lastDate());

        // cannot handle non zero dividends, so don't believe this...
        DiscountFactor dividendDiscount1 =
            procs[0]->dividendYield()->discount(exercise->lastDate());
        DiscountFactor dividendDiscount2 =
            procs[1]->dividendYield()->discount(exercise->lastDate());

        BasketOption::BasketType basketType = arguments_.basketType;

        Real forward1 = procs[0]->stateVariable()->value() *
            dividendDiscount1 / riskFreeDiscount;
        Real forward2 = procs[1]->stateVariable()->value() *
            dividendDiscount2 / riskFreeDiscount;

        switch (basketType) {
          case BasketOption::Max:
            switch (payoff->optionType()) {
              // euro call on a two asset max basket
              case Option::Call:
                results_.value = euroTwoAssetMaxBasketCall(forward1, forward2, strike,
                                                           riskFreeDiscount,
                                                           variance1, variance2,
                                                           rho);

                break;
              // euro put on a two asset max basket
              case Option::Put:
                results_.value = strike * riskFreeDiscount -
                    euroTwoAssetMaxBasketCall(forward1, forward2, 0.0,
                                              riskFreeDiscount,
                                              variance1, variance2, rho) +
                    euroTwoAssetMaxBasketCall(forward1, forward2, strike,
                                              riskFreeDiscount,
                                              variance1, variance2, rho);
                break;
              default:
                QL_FAIL("unknown option type");
            }
            break;
          case BasketOption::Min:
            switch (payoff->optionType()) {
              // euro call on a two asset min basket
              case Option::Call:
                results_.value = euroTwoAssetMinBasketCall(forward1, forward2, strike,
                                                           riskFreeDiscount,
                                                           variance1, variance2,
                                                           rho);
                break;
              // euro put on a two asset min basket
              case Option::Put:
                results_.value = strike * riskFreeDiscount -
                    euroTwoAssetMinBasketCall(forward1, forward2, 0.0,
                                              riskFreeDiscount,
                                              variance1, variance2, rho) +
                    euroTwoAssetMinBasketCall(forward1, forward2, strike,
                                              riskFreeDiscount,
                                              variance1, variance2, rho);
                break;
              default:
                QL_FAIL("unknown option type");
            }
            break;
          default:
            QL_FAIL("unknown type");
        }

    }

}

