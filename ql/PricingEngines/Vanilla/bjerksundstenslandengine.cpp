
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/PricingEngines/Vanilla/bjerksundstenslandengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace {

        CumulativeNormalDistribution cumNormalDist;

        double phi(double S, double gamma, double H, double I,
            double rT, double bT, double variance) {

            double lambda = (-rT + gamma * bT + 0.5 * gamma * (gamma - 1.0)
                * variance);
            double d = -(QL_LOG(S / H) + (bT + (gamma - 0.5) * variance) )
                / QL_SQRT(variance);
            double kappa = 2.0 * bT / variance + (2.0 * gamma - 1.0);
            return QL_EXP(lambda) * QL_POW(S, gamma) * (cumNormalDist(d)
                - QL_POW((I / S), kappa) *
                cumNormalDist(d - 2.0 * QL_LOG(I / S) / QL_SQRT(variance)));
        }


        double americanCallApproximation(double S, double X,
            double rfD, double dD, double variance) {

            double bT = QL_LOG(dD/rfD);
            double rT = QL_LOG(1.0/rfD);

            double Beta = (0.5 - bT/variance) +
                QL_SQRT(QL_POW((bT/variance - 0.5), 2.0) + 2.0 * rT/variance);
            double BInfinity = Beta / (Beta - 1.0) * X;
            // double B0 = QL_MAX(X, QL_LOG(rfD) / QL_LOG(dD) * X);
            double B0 = QL_MAX(X, rT / (rT - bT) * X);
            double ht = -(bT + 2.0 * QL_SQRT(variance)) * B0 /
                (BInfinity - B0);

            // investigate what happen to I for dD->0.0
            double I = B0 + (BInfinity - B0) * (1 - QL_EXP(ht));
            if (S >= I)
                return S - X;
            else {
                // investigate what happen to alpha for dD->0.0
                double alpha = (I - X) * QL_POW(I, (-Beta));
                return alpha * QL_POW(S, Beta)
                    - alpha * phi(S, Beta, I, I, rT, bT, variance)
                    +         phi(S,  1.0, I, I, rT, bT, variance)
                    -         phi(S,  1.0, X, I, rT, bT, variance)
                    -    X *  phi(S,  0.0, I, I, rT, bT, variance)
                    +    X *  phi(S,  0.0, X, I, rT, bT, variance);
            }
        }
    }

    void BjerksundStenslandApproximationEngine::calculate() const {

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

        double variance = 
            arguments_.blackScholesProcess->blackVolatility()->blackVariance(
                                            ex->lastDate(), payoff->strike());
        DiscountFactor dividendDiscount =
            arguments_.blackScholesProcess->dividendYield()->discount(
                                                              ex->lastDate());
        DiscountFactor riskFreeDiscount =
            arguments_.blackScholesProcess->riskFreeRate()->discount(
                                                              ex->lastDate());
        double spot = arguments_.blackScholesProcess->stateVariable()->value();
        double forwardPrice = spot *
            dividendDiscount / riskFreeDiscount;
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
            switch (payoff->optionType()) {
                case Option::Call:
                    results_.value = americanCallApproximation(
                        spot,
                        payoff->strike(),
                        riskFreeDiscount,
                        dividendDiscount,
                        variance);
                    break;
                case Option::Put:
                    // Use put-call simmetry
                    results_.value = americanCallApproximation(
                        payoff->strike(),
                        spot,
                        dividendDiscount,
                        riskFreeDiscount,
                        variance);
                    break;
                default:
                    QL_FAIL("unknown option type");
            }
        } // end of "early exercise can be optimal"
    }

}
