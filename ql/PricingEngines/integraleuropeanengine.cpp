
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
#include <ql/solver1d.hpp>
#include <ql/Math/segmentintegral.hpp>

namespace QuantLib {

    namespace {
        class Payoff : public ObjectiveFunction {
        public:
            Payoff(Option::Type type,
                   Time maturity,
                   double strike,
                   double s0,
                   double sigma,
                   Rate r,
                   Rate q)
           : type_(type), maturity_(maturity), strike_(strike), s0_(s0),
             sigma_(sigma), r_(r), q_(q),
             riskFreeDiscount_(QL_EXP(-r*maturity)),
             drift_((r- q-0.5*sigma*sigma)*maturity),
             variance_(sigma*sigma*maturity) { }

            double operator()(double x) const {

                return riskFreeDiscount_ *
                    ExercisePayoff(type_, s0_*QL_EXP(x), strike_) *
                    QL_EXP(-(x - drift_)*(x -drift_)/(2.0*variance_)) /
                    QL_SQRT(2.0*M_PI*variance_);
            }
            double drift() { return drift_;}
        private:
            Option::Type type_;
            Time maturity_;
            double strike_;
            double s0_;
            double sigma_;
            DiscountFactor riskFreeDiscount_;
            Rate r_, q_;
            double drift_, variance_;
        };
    }

    namespace PricingEngines {

        void IntegralEuropeanEngine::calculate() const {

            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "IntegralEuropeanEngine::calculate() : "
                "not an European Option");

            double vol = arguments_.volTS->blackVol(
                arguments_.maturity, arguments_.strike);
            Rate dividendRate =
                arguments_.dividendTS->zeroYield(arguments_.maturity);
            Rate riskFreeRate =
                arguments_.riskFreeTS->zeroYield(arguments_.maturity);

            Payoff po(arguments_.type, arguments_.maturity, arguments_.strike,
                arguments_.underlying, vol, riskFreeRate,
                dividendRate);
            QuantLib::Math::SegmentIntegral integrator(5000);
            double drift = po.drift();
            double infinity = 10.0*vol*QL_SQRT(arguments_.maturity);
            results_.value = integrator(po, drift-infinity, drift+infinity);
        }

    }

}

