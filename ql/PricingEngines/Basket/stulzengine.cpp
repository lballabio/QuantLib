
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
        double euroTwoAssetMinBasketCall(double forward1, double forward2, 
                                         double strike,
                                         DiscountFactor riskFreeDiscount, 
                                         double variance1, double variance2, 
                                         double rho) {

            double stdDev1 = QL_SQRT(variance1);
            double stdDev2 = QL_SQRT(variance2);

            double variance = variance1 + variance2 - 2 * rho * stdDev1 * stdDev2;
            double stdDev = QL_SQRT(variance);

            double modRho1 = (rho * stdDev2 - stdDev1) / stdDev;
            double modRho2 = (rho * stdDev1 - stdDev2) / stdDev;

            double D1 = (QL_LOG(forward1 / forward2) + 0.5 * variance) / stdDev;

            double alfa, beta, gamma;
            if (strike != 0.0) {
                BivariateCumulativeNormalDistribution bivCNorm = 
                    BivariateCumulativeNormalDistribution(rho);
                BivariateCumulativeNormalDistribution bivCNormMod2 = 
                    BivariateCumulativeNormalDistribution(modRho2);
                BivariateCumulativeNormalDistribution bivCNormMod1 = 
                    BivariateCumulativeNormalDistribution(modRho1);

                double D1_1 = (QL_LOG(forward1/strike) + 0.5 * variance1) / stdDev1;
                double D1_2 = (QL_LOG(forward2/strike) + 0.5 * variance2) / stdDev2;
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
        double euroTwoAssetMaxBasketCall(double forward1, double forward2, 
                                         double strike, 
                                         Rate riskFreeDiscount,
                                         double variance1, double variance2, 
                                         double rho) {

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

        std::vector<boost::shared_ptr<BlackScholesStochasticProcess> > procs = 
            arguments_.blackScholesProcesses;

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "StulzEngine::calculate() : "
                   "not an European Option");

        QL_REQUIRE(procs.size() == 2,
                   "StulzEngine::calculate() : "
                   "not a basket of two stocks");

        boost::shared_ptr<EuropeanExercise> exercise = 
            boost::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise,
                   "StulzEngine::calculate() : "
                   "not an European Option");

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "StulzEngine: "
                   "non-plain payoff given");

        double strike = payoff->strike();

        double variance1 = 
            procs[0]->volTS->blackVariance(exercise->lastDate(), strike);
        double variance2 = 
            procs[1]->volTS->blackVariance(exercise->lastDate(), strike);

        double rho = arguments_.correlation[1][0];

        DiscountFactor riskFreeDiscount =
              procs[0]->riskFreeTS->discount(exercise->lastDate());

        // cannot handle non zero dividends, so don't believe this...
        DiscountFactor dividendDiscount1 =
            procs[0]->dividendTS->discount(exercise->lastDate());
        DiscountFactor dividendDiscount2 =
            procs[1]->dividendTS->discount(exercise->lastDate());

        BasketOption::BasketType basketType = arguments_.basketType;

        double forward1 = procs[0]->stateVariable->value() * dividendDiscount1 / riskFreeDiscount;
        double forward2 = procs[1]->stateVariable->value() * dividendDiscount2 / riskFreeDiscount;

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
              case Option::Straddle:
                QL_FAIL("BasketOption: unsupported option type");
              default:
                QL_FAIL("BasketOption: unknown option type");
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
              case Option::Straddle:
                QL_FAIL("BasketOption: unsupported option type");
              default:
                QL_FAIL("BasketOption: unknown option type");
            }
            break;
          default:
            QL_FAIL("BasketOption: unknown type");
        }

    }

}

