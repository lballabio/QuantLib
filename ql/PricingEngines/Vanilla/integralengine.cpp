
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

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

#include <ql/PricingEngines/Vanilla/integralengine.hpp>
#include <ql/Math/segmentintegral.hpp>

namespace QuantLib {

    namespace {

        class Integrand : std::unary_function<double,double> {
          public:
            Integrand(const boost::shared_ptr<Payoff>& payoff,
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
            boost::shared_ptr<Payoff> payoff_;
            double s0_, drift_, variance_;
        };
    }

    void IntegralEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        boost::shared_ptr<StrikedTypePayoff> payoff = 
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        double variance = 
            arguments_.blackScholesProcess->volTS->blackVariance(
                           arguments_.exercise->lastDate(), payoff->strike());

        double dividendDiscount = 
            arguments_.blackScholesProcess->dividendTS->discount(
                arguments_.exercise->lastDate());
        double riskFreeDiscount = 
            arguments_.blackScholesProcess->riskFreeTS->discount(
                arguments_.exercise->lastDate());
        double drift = QL_LOG(dividendDiscount/riskFreeDiscount)-0.5*variance;

        Integrand f(arguments_.payoff, 
                    arguments_.blackScholesProcess->stateVariable->value(), 
                    drift, variance);
        SegmentIntegral integrator(5000);

        double infinity = 10.0*QL_SQRT(variance);
        results_.value =
            arguments_.blackScholesProcess->riskFreeTS
                ->discount(arguments_.exercise->lastDate()) /
            QL_SQRT(2.0*M_PI*variance) *
            integrator(f, drift-infinity, drift+infinity);
    }

}

