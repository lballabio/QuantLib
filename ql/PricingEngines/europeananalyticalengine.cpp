
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file europeananalyticalengine.cpp
    \brief European option engine using analytic formulas

    \fullpath
    ql/Pricers/%europeananalyticalengine.cpp
*/

// $Id$

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void EuropeanAnalyticalEngine::calculate() const {

            DiscountFactor dividendDiscount =
                QL_EXP(-arguments_.dividendYield*arguments_.residualTime);
            DiscountFactor riskFreeDiscount =
                QL_EXP(-arguments_.riskFreeRate*arguments_.residualTime);

            double stdDev = arguments_.volatility *
                QL_SQRT(arguments_.residualTime);

            static Math::CumulativeNormalDistribution f;

            double D1 = 
                QL_LOG(arguments_.underlying/arguments_.strike)/stdDev +
                stdDev/2.0 +
                (arguments_.riskFreeRate-arguments_.dividendYield) *
                    arguments_.residualTime/stdDev;
            double D2 = D1-stdDev;

            double alpha, beta, NID1;
            switch (arguments_.type) {
              case Option::Call:
                alpha = f(D1);
                beta  = f(D2);
                NID1  = f.derivative(D1);
                break;
              case Option::Put:
                alpha = f(D1)-1.0;
                beta  = f(D2)-1.0;
                NID1  = f.derivative(D1);
                break;
              case Option::Straddle:
                alpha = 2.0*f(D1)-1.0;
                beta  = 2.0*f(D2)-1.0;
                NID1  = 2.0*f.derivative(D1);
                break;
              default:
                throw IllegalArgumentError(
                    "EuropeanAnalyticalEngine::calculate() : "
                    "invalid option type");
            }

            results_.value =
                arguments_.underlying * dividendDiscount * alpha -
                    arguments_.strike * riskFreeDiscount * beta;
            results_.delta = dividendDiscount * alpha;
            results_.gamma = NID1 * dividendDiscount /
                (arguments_.underlying * stdDev);
            results_.theta = - arguments_.underlying * NID1 *
                arguments_.volatility * dividendDiscount /
                (2.0 * QL_SQRT(arguments_.residualTime)) +
                arguments_.dividendYield * arguments_.underlying *
                alpha * dividendDiscount -
                arguments_.riskFreeRate * arguments_.strike *
                riskFreeDiscount * beta;
            results_.rho = arguments_.residualTime * riskFreeDiscount *
                arguments_.strike * beta;
            results_.dividendRho = - arguments_.residualTime *
                dividendDiscount * arguments_.underlying * alpha;
            results_.vega = arguments_.underlying * NID1 *
                dividendDiscount * QL_SQRT(arguments_.residualTime);
        }

    }

}

