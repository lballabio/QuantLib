
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file analyticalvanillaengine.cpp
    \brief Vanilla option engine using analytic formulas

    \fullpath
    ql/Pricers/%analyticalvanillaengine.cpp
*/

// $Id$

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void AnalyticalVanillaEngine::calculate() const {

            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "AnalyticalVanillaEngine::calculate() : "
                "not an European Option");

            double variance = arguments_.volTS->blackVariance(
                arguments_.maturity, arguments_.strike);
            double stdDev = QL_SQRT(variance);
            double vol = arguments_.volTS->blackVol(
                arguments_.maturity, arguments_.strike);

            DiscountFactor dividendDiscount =
                arguments_.dividendTS->discount(arguments_.maturity);
            Rate dividendRate =
                arguments_.dividendTS->zeroYield(arguments_.maturity);

            DiscountFactor riskFreeDiscount =
                arguments_.riskFreeTS->discount(arguments_.maturity);
            Rate riskFreeRate =
                arguments_.riskFreeTS->zeroYield(arguments_.maturity);
            double forwardPrice = arguments_.underlying *
                dividendDiscount / riskFreeDiscount;

            double fD1, fD2, fderD1;
            if (variance>0.0) {
                static Math::CumulativeNormalDistribution f;
                double D1 = (QL_LOG(forwardPrice/arguments_.strike) +
                             0.5 * variance) / stdDev;
                double D2 = D1-stdDev;
                fD1 = f(D1);
                fD2 = f(D2);
                fderD1 = f.derivative(D1);
            } else {
                stdDev = QL_EPSILON;
                fderD1 = 0.0;
                if (forwardPrice>arguments_.strike) {
                    fD1 = 1.0;
                    fD2 = 1.0;
                } else {
                    fD1 = 0.0;
                    fD2 = 0.0;
                }
            }

            double alpha, beta, NID1;
            switch (arguments_.type) {
              case Option::Call:
                alpha = fD1;
                beta  = fD2;
                NID1  = fderD1;
                break;
              case Option::Put:
                alpha = fD1-1.0;
                beta  = fD2-1.0;
                NID1  = fderD1;
                break;
              case Option::Straddle:
                alpha = 2.0*fD1-1.0;
                beta  = 2.0*fD2-1.0;
                NID1  = 2.0*fderD1;
                break;
              default:
                throw IllegalArgumentError(
                    "EuropeanAnalyticalEngine::calculate() : "
                    "invalid option type");
            }


            results_.value = riskFreeDiscount *
                (forwardPrice * alpha - arguments_.strike *  beta);
            results_.delta = dividendDiscount * alpha;
//            results_.deltaForward = riskFreeDiscount * alpha;
            results_.gamma = NID1 * dividendDiscount /
                (arguments_.underlying * stdDev);
            results_.theta = riskFreeRate * results_.value
                -(riskFreeRate - dividendRate) 
                * arguments_.underlying * results_.delta
                - 0.5 * vol * vol * arguments_.underlying 
                * arguments_.underlying * results_.gamma;
            results_.rho = arguments_.maturity * riskFreeDiscount *
                arguments_.strike * beta;
            results_.dividendRho = - arguments_.maturity *
                dividendDiscount * arguments_.underlying * alpha;
            results_.vega = arguments_.underlying * NID1 *
                dividendDiscount * QL_SQRT(arguments_.maturity);

            results_.strikeSensitivity = - riskFreeDiscount * beta;
        }

    }

}

