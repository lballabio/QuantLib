
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

/*! \file integralengines.cpp
    \brief Option engines using the integral approach
*/

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/Math/segmentintegral.hpp>

#include <ql/handle.hpp>

namespace QuantLib {

    namespace {
        class Integrand : std::unary_function<double,double> {
        public:
            Integrand(const Handle<Payoff>& payoff,
                      double s0,
                      double drift,
                      double variance)
           : payoff_(payoff), s0_(s0), drift_(drift), variance_(variance) {}
            double operator()(double x) const {
                double temp = s0_ * QL_EXP(x);
                double result = (*payoff_)(temp);
                return result *
                    QL_EXP(-(x - drift_)*(x -drift_)/(2.0*variance_)) ;
            }
        private:
            Handle<Payoff> payoff_;
            double s0_, drift_, variance_;
        };
    }

    namespace PricingEngines {

        void IntegralEngine::calculate() const {

            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "IntegralEuropeanEngine::calculate() : "
                "not an European Option");

            Handle<PlainPayoff> payoff = arguments_.payoff;

            double variance = arguments_.volTS->blackVariance(
                arguments_.maturity, payoff->strike());

            Rate dividendRate =
                arguments_.dividendTS->zeroYield(arguments_.maturity);
            Rate riskFreeRate =
                arguments_.riskFreeTS->zeroYield(arguments_.maturity);
            double drift = (riskFreeRate - dividendRate) * arguments_.maturity
                - 0.5 * variance;

            Integrand f(arguments_.payoff, arguments_.underlying, 
                        drift, variance);
            QuantLib::Math::SegmentIntegral integrator(5000);

            double infinity = 10.0*QL_SQRT(variance);
            results_.value =
                arguments_.riskFreeTS->discount(arguments_.maturity) /
                QL_SQRT(2.0*M_PI*variance) *
                integrator(f, drift-infinity, drift+infinity);
        }

    }

}

