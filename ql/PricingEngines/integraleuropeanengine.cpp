
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

/*! \file integraleuropeanengine.cpp
    \brief European option engine using integral approach

    \fullpath
    ql/PricingEngines/%integraleuropeanengine.cpp
*/

// $Id$

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/Math/segmentintegral.hpp>

namespace QuantLib {

    namespace {
        class WeightedPayoff : std::unary_function<double,double> {
        public:
            WeightedPayoff(Payoff payoff,
                   double s0,
                   double drift,
                   double variance)
           : payoff_(payoff), s0_(s0), drift_(drift), variance_(variance) {}
            double operator()(double x) const {
                return payoff_(s0_*QL_EXP(x)) *
                    QL_EXP(-(x - drift_)*(x -drift_)/(2.0*variance_)) ;
            }
        private:
            Payoff payoff_;
            double s0_;
            double drift_, variance_;
        };
    }

    namespace PricingEngines {

        void IntegralEuropeanEngine::calculate() const {

            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "IntegralEuropeanEngine::calculate() : "
                "not an European Option");

            double variance = arguments_.volTS->blackVariance(
                arguments_.maturity, arguments_.payoff.strike());

            Rate dividendRate =
                arguments_.dividendTS->zeroYield(arguments_.maturity);
            Rate riskFreeRate =
                arguments_.riskFreeTS->zeroYield(arguments_.maturity);
            double drift = (riskFreeRate - dividendRate) * arguments_.maturity
                - 0.5 * variance;

            WeightedPayoff po(arguments_.payoff, arguments_.underlying,
                drift, variance);
            QuantLib::Math::SegmentIntegral integrator(5000);

            double infinity = 10.0*QL_SQRT(variance);
            results_.value =
                arguments_.riskFreeTS->discount(arguments_.maturity) /
                QL_SQRT(2.0*M_PI*variance) *
                integrator(po, drift-infinity, drift+infinity);
        }

    }

}

