
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

#include <ql/PricingEngines/Vanilla/juquadraticengine.hpp>
#include <ql/PricingEngines/Vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    /*  An Approximate Formula for Pricing American Options
        Journal of Derivatives Winter 1999
        Ju, N.      
    */

    void JuQuadraticApproximationEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American Option");

        boost::shared_ptr<AmericanExercise> ex = 
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(!ex->payoffAtExpiry(),
                   "payoff at expiry not handled");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance = 
            arguments_.blackScholesProcess->blackVolatility()->blackVariance(
                                            ex->lastDate(), payoff->strike());
        DiscountFactor dividendDiscount =
            arguments_.blackScholesProcess->dividendYield()->discount(
                                                              ex->lastDate());
        DiscountFactor riskFreeDiscount =
            arguments_.blackScholesProcess->riskFreeRate()->discount(
                                                              ex->lastDate());
        Real spot = arguments_.blackScholesProcess->stateVariable()->value();
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;
        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);

        if (dividendDiscount>=1.0 && payoff->optionType()==Option::Call) {
            // early exercise never optimal
            results_.value        = black.value();
            results_.delta        = black.delta(spot);
            results_.deltaForward = black.deltaForward();
            results_.elasticity   = black.elasticity(spot);
            results_.gamma        = black.gamma(spot);

            Time t =
                arguments_.blackScholesProcess->riskFreeRate()
                ->dayCounter().yearFraction(arguments_.blackScholesProcess
                                            ->riskFreeRate()->referenceDate(),
                                            arguments_.exercise->lastDate());
            results_.rho = black.rho(t);

            t = arguments_.blackScholesProcess->dividendYield()
                ->dayCounter().yearFraction(arguments_.blackScholesProcess
                                            ->dividendYield()->referenceDate(),
                                            arguments_.exercise->lastDate());
            results_.dividendRho = black.dividendRho(t);

            t = arguments_.blackScholesProcess->blackVolatility()
                ->dayCounter().yearFraction(arguments_.blackScholesProcess
                                         ->blackVolatility()->referenceDate(),
                                            arguments_.exercise->lastDate());
            results_.vega        = black.vega(t);
            results_.theta       = black.theta(spot, t);
            results_.thetaPerDay = black.thetaPerDay(spot, t);

            results_.strikeSensitivity  = black.strikeSensitivity();
            results_.itmCashProbability = black.itmCashProbability();
        } else {
            // early exercise can be optimal 
            CumulativeNormalDistribution cumNormalDist;
            NormalDistribution normalDist;

            Real tolerance = 1e-6;
            Real Sk = BaroneAdesiWhaleyApproximationEngine::criticalPrice(
                payoff, riskFreeDiscount, dividendDiscount, variance,
                tolerance);

            Real forwardSk = Sk * dividendDiscount / riskFreeDiscount;
            // the following 3 assignements are not used - Nando
            // ???????????????????????????
            /*
            Real d1 = (QL_LOG(forwardSk/payoff->strike()) + 0.5*variance)
                /QL_SQRT(variance);
            Real n = 2.0*QL_LOG(dividendDiscount/riskFreeDiscount)/variance;
            Real K = -2.0*QL_LOG(riskFreeDiscount)/
                (variance*(1.0-riskFreeDiscount));            

            */

            Real alpha = -2.0*QL_LOG(riskFreeDiscount)/(variance);
            Real beta = 2.0*QL_LOG(dividendDiscount/riskFreeDiscount)/
                                                (variance);
            Real h = 1 - riskFreeDiscount;
            Real phi;
            switch (payoff->optionType()) {
                case Option::Call:
                    phi = 1;
                    break;
                case Option::Put:
                    phi = -1;
                    break;
                default:
                  QL_FAIL("unknown option type");
            }
            Real temp_root = QL_SQRT ((beta-1)*(beta-1) + (4*alpha)/h);
            Real lambda = (-(beta-1) + phi * temp_root) / 2;
            Real lambda_prime = - phi * alpha / (h*h * temp_root);

            BlackFormula 
                    black_Sk(forwardSk, riskFreeDiscount, variance, payoff);
            Real hA = phi * (Sk - payoff->strike()) - black_Sk.value();

            Real d1_Sk = (QL_LOG(forwardSk/payoff->strike()) + 0.5*variance)
                /QL_SQRT(variance);            
            Real d2_Sk = d1_Sk - QL_SQRT(variance);
            Real part1 = forwardSk * normalDist(d1_Sk) / 
                                        (alpha * QL_SQRT(variance));
            Real part2 = - phi * forwardSk * cumNormalDist(phi * d1_Sk) * 
                        QL_LOG (dividendDiscount) / QL_LOG(riskFreeDiscount);
            Real part3 = + phi * payoff->strike() * cumNormalDist(phi * d2_Sk);
            Real V_E_h = part1 + part2 + part3;                                

            Real b = (1-h) * alpha * lambda_prime / (2*(2*lambda + beta - 1));
            Real c = - ((1 - h) * alpha / (2 * lambda + beta - 1)) * 
                (V_E_h / (hA) + 1 / h + lambda_prime / (2*lambda + beta - 1));
            Real temp_spot_ratio = QL_LOG(spot / Sk);
            Real chi = temp_spot_ratio * (b * temp_spot_ratio + c);            

            if (phi*(Sk-spot) > 0) {
                results_.value = black.value() + 
                    hA * QL_POW((spot/Sk), lambda) / (1 - chi);
            } else {
                results_.value = phi * (spot - payoff->strike());
            }

            Real temp_chi_prime = (2 * b / spot) * QL_LOG (spot/Sk);
            Real chi_prime = temp_chi_prime + c / spot;
            Real chi_double_prime = 2*b/(spot*spot) 
                                    - temp_chi_prime / spot 
                                    - c / (spot*spot);
            results_.delta = phi * dividendDiscount * cumNormalDist (phi * d1_Sk)
                + (lambda / (spot * (1 - chi)) + chi_prime / ((1 - chi)*(1 - chi))) * 
                (phi * (Sk - payoff->strike()) - black_Sk.value()) * QL_POW((spot/Sk), lambda);

            results_.gamma = phi * dividendDiscount * normalDist (phi*d1_Sk) /
                                        (spot * QL_SQRT(variance))
                           + (2 * lambda * chi_prime / (spot * (1 - chi) * (1 - chi))
                              + 2 * chi_prime * chi_prime / ((1 - chi) * (1 - chi) * (1 - chi))
                              + chi_double_prime / ((1 - chi) * (1 - chi))
                              + lambda * (1 - lambda) / (spot * spot * (1 - chi)))
                            * (phi * (Sk - payoff->strike()) - black_Sk.value()) 
                                 * QL_POW((spot/Sk), lambda);   

        } // end of "early exercise can be optimal"
    }

}
