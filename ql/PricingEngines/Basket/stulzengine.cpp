
/*
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

/*! \file stulzengine.cpp
    \brief European 2D Basket option formulae

    "Options on the Minimum or the Maximum of Two Risky Assets", 
        Rene Stulz, 
        Journal of Financial Ecomomics (1982) 10, 161-185.
*/

#include <ql/PricingEngines/Basket/stulzengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    namespace {

        // calculate the value of euro min basket call
        double euroTwoAssetMinBasketCall(double s1_0, double s2_0, 
                                         double strike, 
                                         DiscountFactor riskFreeDiscount, 
                                         Rate intRate, 
                                         double vol_s1, double vol_s2, 
                                         double rho, Time expiry) {

            double rootExpiry = QL_SQRT(expiry);

            double gamma1;
            double gamma2;
            if (strike != 0.0) {
                gamma1 = (QL_LOG(s1_0 / strike) + 
                          (intRate - 0.5 * vol_s1 * vol_s1) * expiry) / 
                    (vol_s1 * rootExpiry);
                gamma2 = (QL_LOG(s2_0 / strike) + 
                          (intRate - 0.5 * vol_s2 * vol_s2) * expiry) / 
                    (vol_s2 * rootExpiry);
            } else {
                gamma1 = 1000;
                gamma2 = 1000;
            }

            double sigma_squared = vol_s1 * vol_s1 + vol_s2 * vol_s2 - 
                2 * rho * vol_s1 * vol_s2;
            double sigma = QL_SQRT(sigma_squared);

            double a1 = gamma1 + vol_s1 * rootExpiry;
            double b1 = (QL_LOG(s2_0 / s1_0) - 
                         0.5 * sigma_squared * expiry) / 
                (sigma * rootExpiry);
            double a2 = gamma2 + vol_s2 * rootExpiry;
            double b2 = (QL_LOG(s1_0 / s2_0) - 
                         0.5 * sigma_squared * expiry) / 
                (sigma * rootExpiry);
            double modRho1 = (rho * vol_s2 - vol_s1) / sigma;
            double modRho2 = (rho * vol_s1 - vol_s2) / sigma;

            BivariateCumulativeNormalDistribution bivCNorm = 
                BivariateCumulativeNormalDistribution(rho);
            BivariateCumulativeNormalDistribution bivCNormMod2 = 
                BivariateCumulativeNormalDistribution(modRho2);
            BivariateCumulativeNormalDistribution bivCNormMod1 = 
                BivariateCumulativeNormalDistribution(modRho1);

            return s1_0 * bivCNormMod1(a1, b1) + 
                s2_0 * bivCNormMod2(a2, b2) - 
                strike * riskFreeDiscount * bivCNorm(gamma1, gamma2);

        }

        // calculate the value of euro max basket call
        double euroTwoAssetMaxBasketCall(double s1_0, double s2_0, 
                                         double strike, 
                                         Rate riskFreeDiscount, Rate intRate, 
                                         double vol_s1, double vol_s2, 
                                         double rho, Time expiry) {

            double dividendDiscount = 1.0;

            double forwardPrice1 = s1_0 * dividendDiscount / riskFreeDiscount;
            double forwardPrice2 = s2_0 * dividendDiscount / riskFreeDiscount;

            Handle<StrikedTypePayoff> payoff(new
                PlainVanillaPayoff(Option::Call, strike));

            BlackFormula black1(forwardPrice1, riskFreeDiscount, 
                                vol_s1*vol_s1, payoff);
            BlackFormula black2(forwardPrice2, riskFreeDiscount, 
                                vol_s2*vol_s2, payoff);

            return black1.value() + black2.value() - 
                euroTwoAssetMinBasketCall(s1_0, s2_0, strike, 
                                          riskFreeDiscount, intRate, 
                                          vol_s1, vol_s2, rho, expiry);
        }
    }

    void StulzEngine::calculate() const {

        std::vector< Handle<BlackScholesStochasticProcess> > procs = 
            arguments_.blackScholesProcesses;

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "StulzEngine::calculate() : "
                   "not an European Option");

        QL_REQUIRE(procs.size() == 2,
                   "StulzEngine::calculate() : "
                   "not a basket of two stocks");

        #if defined(HAVE_BOOST)
        Handle<EuropeanExercise> exercise = 
            boost::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise,
                   "StulzEngine::calculate() : "
                   "not an European Option");
        #else
        Handle<EuropeanExercise> exercise = arguments_.exercise;
        #endif

        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "StulzEngine: "
                   "non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif

        double s1_0 = procs[0]->stateVariable->value();
        double s2_0 = procs[1]->stateVariable->value();

        double strike = payoff->strike();

        double vol_s1 = 
            procs[0]->volTS->blackVol(exercise->lastDate(), strike);
        double vol_s2 = 
            procs[1]->volTS->blackVol(exercise->lastDate(), strike);

        double rho = arguments_.correlation;

        DiscountFactor riskFreeDiscount =
              procs[0]->riskFreeTS->discount(exercise->lastDate());

        // cannot handle non zero dividends, so don't believe this...
        DiscountFactor dividendDiscount =
            procs[0]->dividendTS->discount(exercise->lastDate());

        QL_REQUIRE(dividendDiscount == 1,
                   "StulzEngine: "
                   "Dividends should be zero");

        Time expiry = procs[0]->riskFreeTS->dayCounter().yearFraction(
                procs[0]->riskFreeTS->referenceDate(),
                exercise->lastDate());

        double intRate = procs[0]->riskFreeTS->zeroYield(exercise->lastDate());

        BasketOption::BasketType basketType = arguments_.basketType;

        switch (basketType) {
          case BasketOption::Max:
            switch (payoff->optionType()) {
              // euro call on a two asset max basket 
              case Option::Call:
                results_.value = euroTwoAssetMaxBasketCall(s1_0, s2_0, strike, 
                                                           riskFreeDiscount, 
                                                           intRate, 
                                                           vol_s1, vol_s2, 
                                                           rho, expiry);

                break;
              // euro put on a two asset max basket 
              case Option::Put:
                results_.value = strike * riskFreeDiscount -
                    euroTwoAssetMaxBasketCall(s1_0, s2_0, 0.0,
                                              riskFreeDiscount, intRate, 
                                              vol_s1, vol_s2, rho, expiry) + 
                    euroTwoAssetMaxBasketCall(s1_0, s2_0, strike, 
                                              riskFreeDiscount, intRate, 
                                              vol_s1, vol_s2, rho, expiry);
                break;
              case Option::Straddle:
                throw Error("BasketOption: unsupported option type");
            }
            break;
          case BasketOption::Min:
            switch (payoff->optionType()) {
              // euro call on a two asset min basket 
              case Option::Call:
                results_.value = euroTwoAssetMinBasketCall(s1_0, s2_0, strike, 
                                                           riskFreeDiscount, 
                                                           intRate, 
                                                           vol_s1, vol_s2, 
                                                           rho, expiry);
                break;
              // euro put on a two asset min basket 
              case Option::Put:
                results_.value = strike * riskFreeDiscount -
                    euroTwoAssetMinBasketCall(s1_0, s2_0, 0.0, 
                                              riskFreeDiscount, intRate, 
                                              vol_s1, vol_s2, rho, expiry) +
                    euroTwoAssetMinBasketCall(s1_0, s2_0, strike, 
                                              riskFreeDiscount, intRate, 
                                              vol_s1, vol_s2, rho, expiry);
                break;
              case Option::Straddle:
                throw Error("BasketOption: unsupported option type");
            }
            break;
          default:
            throw Error("BasketOption: unknown type");
        }

    }

}

