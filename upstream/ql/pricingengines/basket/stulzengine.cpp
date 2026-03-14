/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/basket/stulzengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <utility>

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
                auto bivCNorm =
                    BivariateCumulativeNormalDistribution(rho);
                auto bivCNormMod2 =
                    BivariateCumulativeNormalDistribution(modRho2);
                auto bivCNormMod1 =
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

            ext::shared_ptr<StrikedTypePayoff> payoff(new
                PlainVanillaPayoff(Option::Call, strike));

            Real black1 = blackFormula(payoff->optionType(), payoff->strike(),
                forward1, std::sqrt(variance1)) * riskFreeDiscount;

            Real black2 = blackFormula(payoff->optionType(), payoff->strike(),
                forward2, std::sqrt(variance2)) * riskFreeDiscount;

            return black1 + black2 -
                euroTwoAssetMinBasketCall(forward1, forward2, strike,
                                          riskFreeDiscount,
                                          variance1, variance2, rho);
        }
    }

    StulzEngine::StulzEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
                             ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
                             Real correlation)
    : process1_(std::move(process1)), process2_(std::move(process2)), rho_(correlation) {
        registerWith(process1_);
        registerWith(process2_);
    }

    void StulzEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European Option");

        ext::shared_ptr<BasketPayoff> basket_payoff =
            ext::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);

        ext::shared_ptr<MinBasketPayoff> min_basket =
            ext::dynamic_pointer_cast<MinBasketPayoff>(arguments_.payoff);

        ext::shared_ptr<MaxBasketPayoff> max_basket =
            ext::dynamic_pointer_cast<MaxBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(min_basket || max_basket, "unknown basket type");

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(basket_payoff->basePayoff());
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real strike = payoff->strike();

        Real variance1 = process1_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), strike);
        Real variance2 = process2_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), strike);

        DiscountFactor riskFreeDiscount =
            process1_->riskFreeRate()->discount(exercise->lastDate());

        // cannot handle non zero dividends, so don't believe this...
        DiscountFactor dividendDiscount1 =
            process1_->dividendYield()->discount(exercise->lastDate());
        DiscountFactor dividendDiscount2 =
            process2_->dividendYield()->discount(exercise->lastDate());

        Real forward1 = process1_->stateVariable()->value() *
            dividendDiscount1 / riskFreeDiscount;
        Real forward2 = process2_->stateVariable()->value() *
            dividendDiscount2 / riskFreeDiscount;

        if (max_basket != nullptr) {
            switch (payoff->optionType()) {
              // euro call on a two asset max basket
              case Option::Call:
                results_.value =
                    euroTwoAssetMaxBasketCall(forward1, forward2, strike,
                                              riskFreeDiscount,
                                              variance1, variance2,
                                              rho_);

                break;
              // euro put on a two asset max basket
              case Option::Put:
                results_.value = strike * riskFreeDiscount -
                    euroTwoAssetMaxBasketCall(forward1, forward2, 0.0,
                                              riskFreeDiscount,
                                              variance1, variance2, rho_) +
                    euroTwoAssetMaxBasketCall(forward1, forward2, strike,
                                              riskFreeDiscount,
                                              variance1, variance2, rho_);
                break;
              default:
                QL_FAIL("unknown option type");
            }
        } else if (min_basket != nullptr) {
            switch (payoff->optionType()) {
              // euro call on a two asset min basket
              case Option::Call:
                results_.value =
                    euroTwoAssetMinBasketCall(forward1, forward2, strike,
                                              riskFreeDiscount,
                                              variance1, variance2,
                                              rho_);
                break;
              // euro put on a two asset min basket
              case Option::Put:
                results_.value = strike * riskFreeDiscount -
                    euroTwoAssetMinBasketCall(forward1, forward2, 0.0,
                                              riskFreeDiscount,
                                              variance1, variance2, rho_) +
                    euroTwoAssetMinBasketCall(forward1, forward2, strike,
                                              riskFreeDiscount,
                                              variance1, variance2, rho_);
                break;
              default:
                QL_FAIL("unknown option type");
            }
        } else {
            QL_FAIL("unknown type");
        }
    }

}

