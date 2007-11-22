/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/pricingengines/vanilla/integralengine.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    namespace {

        class Integrand : std::unary_function<Real,Real> {
          public:
            Integrand(const boost::shared_ptr<Payoff>& payoff,
                      Real s0,
                      Rate drift,
                      Real variance)
           : payoff_(payoff), s0_(s0), drift_(drift), variance_(variance) {}
            Real operator()(Real x) const {
                Real temp = s0_ * std::exp(x);
                Real result = (*payoff_)(temp);
                return result *
                    std::exp(-(x - drift_)*(x -drift_)/(2.0*variance_)) ;
            }
          private:
            boost::shared_ptr<Payoff> payoff_;
            Real s0_;
            Rate drift_;
            Real variance_;
        };
    }

    IntegralEngine::IntegralEngine(
              const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void IntegralEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance =
            process_->blackVolatility()->blackVariance(
                           arguments_.exercise->lastDate(), payoff->strike());

        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(
                                             arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(arguments_.exercise->lastDate());
        Rate drift = std::log(dividendDiscount/riskFreeDiscount)-0.5*variance;

        Integrand f(arguments_.payoff,
                    process_->stateVariable()->value(),
                    drift, variance);
        SegmentIntegral integrator(5000);

        Real infinity = 10.0*std::sqrt(variance);
        results_.value =
            process_->riskFreeRate()->discount(
                                            arguments_.exercise->lastDate()) /
            std::sqrt(2.0*M_PI*variance) *
            integrator(f, drift-infinity, drift+infinity);
    }

}

