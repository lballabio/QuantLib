
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file europeanengine.cpp
    \brief analytic pricing engine for European options

    \fullpath
    ql/Pricers/%europeanengine.cpp
*/

// $Id$

#include <ql/Pricers/europeanengine.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace Pricers {

        namespace {
            Math::CumulativeNormalDistribution f;
        }

        void EuropeanEngine::calculate() const {
            double alpha, beta;
            double stdDev = parameters_.volatility *
                QL_SQRT(parameters_.residualTime);
            double D1 =
                QL_LOG(parameters_.underlying/parameters_.strike)/stdDev +
                stdDev/2.0 +
                (parameters_.riskFreeRate-parameters_.dividendYield) *
                    parameters_.residualTime/stdDev;
            double D2 = D1-stdDev;
            double NID1;
            DiscountFactor dividendDiscount =
                QL_EXP(-parameters_.dividendYield*parameters_.residualTime);
            DiscountFactor riskFreeDiscount =
                QL_EXP(-parameters_.riskFreeRate*parameters_.residualTime);

            switch (parameters_.type) {
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
                throw IllegalArgumentError("invalid option type");
            }

            results_.value =
                parameters_.underlying * dividendDiscount * alpha -
                    parameters_.strike * riskFreeDiscount * beta;
            results_.delta = dividendDiscount * alpha;
            results_.gamma = NID1 * dividendDiscount /
                (parameters_.underlying * stdDev);
            results_.theta = - parameters_.underlying * NID1 *
                parameters_.volatility * dividendDiscount /
                (2.0 * QL_SQRT(parameters_.residualTime)) +
                parameters_.dividendYield * parameters_.underlying *
                alpha * dividendDiscount -
                parameters_.riskFreeRate * parameters_.strike *
                riskFreeDiscount * beta;
            results_.rho = parameters_.residualTime * riskFreeDiscount *
                parameters_.strike * beta;
            results_.dividendRho = - parameters_.residualTime *
                dividendDiscount * parameters_.underlying * alpha;
            results_.vega = parameters_.underlying * NID1 *
                dividendDiscount * QL_SQRT(parameters_.residualTime);
        }

    }

}

