
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

/*! \file bjerksundstenslandengine.cpp
    \brief American option engine using Bjerksund and Stensland approximation
*/

#include <ql/PricingEngines/Vanilla/vanillaengines.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {


    namespace {

        CumulativeNormalDistribution CND;

        double phi(double S, double gamma, double H, double I,
            double rfD, double dD, double variance) {

            double bT = QL_LOG(dD/rfD);
            double rT = QL_LOG(1.0/rfD);

            double lambda = (-rT + gamma * bT + 0.5 * gamma * (gamma - 1.0) * variance);
            double d = -(QL_LOG(S / H) + (bT + (gamma - 0.5) * variance) ) / QL_SQRT(variance);
            double kappa = 2.0 * bT / variance + (2.0 * gamma - 1.0);
            return QL_EXP(lambda) * QL_POW(S, gamma) * (CND(d)
                - QL_POW((I / S), kappa) * CND(d - 2.0 * QL_LOG(I / S) / QL_SQRT(variance)));
        }


        double americanCallApproximation(double S, double X,
            double rfD, double dD, double variance) {

            double bT = QL_LOG(dD/rfD);
            double rT = QL_LOG(1.0/rfD);
    
            double Beta = (0.5 - bT/variance) + QL_SQRT(QL_POW((bT/variance - 0.5), 2.0) + 2.0 * rT/variance);
            double BInfinity = Beta / (Beta - 1.0) * X;
            double B0 = QL_MAX(X, rT / (rT - bT) * X);
            double ht = -(bT + 2.0 * QL_SQRT(variance)) * B0 / (BInfinity - B0);
            double I = B0 + (BInfinity - B0) * (1 - QL_EXP(ht));
            double alpha = (I - X) * QL_POW(I, (-Beta));
            if (S >= I)
                return S - X;
            else
                return alpha * QL_POW(S, Beta)
                    - alpha * phi(S, Beta, I, I, rfD, dD, variance)
                    +         phi(S,  1.0, I, I, rfD, dD, variance)
                    -         phi(S,  1.0, X, I, rfD, dD, variance)
                    -    X *  phi(S,  0.0, I, I, rfD, dD, variance)
                    +    X *  phi(S,  0.0, X, I, rfD, dD, variance);
        }
    }

    void BjerksundStenslandApproximationEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "BjerksundStenslandApproximationEngine::calculate() : "
                   "not an American Option");

        #if defined(HAVE_BOOST)
        Handle<AmericanExercise> ex = 
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex,
                   "BjerksundStenslandApproximationEngine: non-American exercise given");
        #else
        Handle<AmericanExercise> ex = arguments_.exercise;
        #endif
        QL_REQUIRE(!ex->payoffAtExpiry(),
                   "BjerksundStenslandApproximationEngine::calculate() : "
                   "payoff at expiry not handled");

        #if defined(HAVE_BOOST)
        Handle<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticEuropeanEngine: non-striked payoff given");
        #else
        Handle<StrikedTypePayoff> payoff = arguments_.payoff;
        #endif

        double variance = arguments_.volTS->blackVariance(
            ex->lastDate(), payoff->strike());
        DiscountFactor dividendDiscount =
            arguments_.dividendTS->discount(ex->lastDate());
        DiscountFactor riskFreeDiscount =
            arguments_.riskFreeTS->discount(ex->lastDate());
        double forwardPrice = arguments_.underlying *
            dividendDiscount / riskFreeDiscount;
        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);

        if (dividendDiscount>=1.0 && payoff->optionType()==Option::Call) {
            // early exercise never optimal
            results_.value        = black.value();
            results_.delta        = black.delta(arguments_.underlying);
            results_.deltaForward = black.deltaForward();
            results_.elasticity   = black.elasticity(arguments_.underlying);
            results_.gamma        = black.gamma(arguments_.underlying);

            Time t = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(),
                arguments_.exercise->lastDate());
            results_.rho = black.rho(t);

            t = arguments_.dividendTS->dayCounter().yearFraction(
                arguments_.dividendTS->referenceDate(),
                arguments_.exercise->lastDate());
            results_.dividendRho = black.dividendRho(t);

            t = arguments_.volTS->dayCounter().yearFraction(
                arguments_.volTS->referenceDate(),
                arguments_.exercise->lastDate());
            results_.vega        = black.vega(t);
            results_.theta       = black.theta(arguments_.underlying, t);
            results_.thetaPerDay = black.thetaPerDay(arguments_.underlying, t);

            results_.strikeSensitivity = black.strikeSensitivity();
            results_.itmProbability    = black.itmProbability();
        } else {
            // early exercise can be optimal 
            switch (payoff->optionType()) {
                case Option::Call:
                    results_.value = americanCallApproximation(
                        arguments_.underlying, payoff->strike(),
                        riskFreeDiscount, dividendDiscount, variance);
                    break;
                case Option::Put:
                    // Use the Bjerksund and Stensland put-call transformation
                    results_.value = americanCallApproximation(
                        payoff->strike(), arguments_.underlying,
                        riskFreeDiscount/dividendDiscount, 1.0/dividendDiscount, variance);
                    break;
                default:
                    throw Error("BaroneAdesiWhaleyApproximationEngine::"
                        "calculate() :"
                        "unknown option type");
            }
        } // end of "early exercise can be optimal"


    }

}
