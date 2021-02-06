/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Ferdinando Ametrano
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

#include <ql/instruments/oneassetoption.hpp>
#include <ql/exercise.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    OneAssetOption::OneAssetOption(
        const ext::shared_ptr<Payoff>& payoff,
        const ext::shared_ptr<Exercise>& exercise)
    : Option(payoff, exercise) {}

    bool OneAssetOption::isExpired() const {
        return detail::simple_event(exercise_->lastDate()).hasOccurred();
    }

    Real OneAssetOption::delta() const {
        calculate();
        QL_REQUIRE(delta_ != Null<Real>(), "delta not provided");
        return delta_;
    }

    Real OneAssetOption::deltaForward() const {
        calculate();
        QL_REQUIRE(deltaForward_ != Null<Real>(),
                   "forward delta not provided");
        return deltaForward_;
    }

    Real OneAssetOption::elasticity() const {
        calculate();
        QL_REQUIRE(elasticity_ != Null<Real>(), "elasticity not provided");
        return elasticity_;
    }

    Real OneAssetOption::gamma() const {
        calculate();
        QL_REQUIRE(gamma_ != Null<Real>(), "gamma not provided");
        return gamma_;
    }

    Real OneAssetOption::theta() const {
        calculate();
        QL_REQUIRE(theta_ != Null<Real>(), "theta not provided");
        return theta_;
    }

    Real OneAssetOption::thetaPerDay() const {
        calculate();
        QL_REQUIRE(thetaPerDay_ != Null<Real>(), "theta per-day not provided");
        return thetaPerDay_;
    }

    Real OneAssetOption::vega() const {
        calculate();
        QL_REQUIRE(vega_ != Null<Real>(), "vega not provided");
        return vega_;
    }

    Real OneAssetOption::rho() const {
        calculate();
        QL_REQUIRE(rho_ != Null<Real>(), "rho not provided");
        return rho_;
    }

    Real OneAssetOption::dividendRho() const {
        calculate();
        QL_REQUIRE(dividendRho_ != Null<Real>(), "dividend rho not provided");
        return dividendRho_;
    }

    Real OneAssetOption::strikeSensitivity() const {
        calculate();
        QL_REQUIRE(strikeSensitivity_ != Null<Real>(),
                   "strike sensitivity not provided");
        return strikeSensitivity_;
    }

    Real OneAssetOption::itmCashProbability() const {
        calculate();
        QL_REQUIRE(itmCashProbability_ != Null<Real>(),
                   "in-the-money cash probability not provided");
        return itmCashProbability_;
    }

    void OneAssetOption::setupExpired() const {
        Option::setupExpired();
        delta_ = deltaForward_ = elasticity_ = gamma_ = theta_ =
            thetaPerDay_ = vega_ = rho_ = dividendRho_ =
            strikeSensitivity_ = itmCashProbability_ = 0.0;
    }

    void OneAssetOption::fetchResults(const PricingEngine::results* r) const {
        Option::fetchResults(r);
        const auto* results = dynamic_cast<const Greeks*>(r);
        QL_ENSURE(results != nullptr, "no greeks returned from pricing engine");
        /* no check on null values - just copy.
           this allows:
           a) to decide in derived options what to do when null
           results are returned (throw? numerical calculation?)
           b) to implement slim engines which only calculate the
           value---of course care must be taken not to call
           the greeks methods when using these.
        */
        delta_          = results->delta;
        gamma_          = results->gamma;
        theta_          = results->theta;
        vega_           = results->vega;
        rho_            = results->rho;
        dividendRho_    = results->dividendRho;

        const auto* moreResults = dynamic_cast<const MoreGreeks*>(r);
        QL_ENSURE(moreResults != nullptr, "no more greeks returned from pricing engine");
        /* no check on null values - just copy.
           this allows:
           a) to decide in derived options what to do when null
           results are returned (throw? numerical calculation?)
           b) to implement slim engines which only calculate the
           value---of course care must be taken not to call
           the greeks methods when using these.
        */
        deltaForward_       = moreResults->deltaForward;
        elasticity_         = moreResults->elasticity;
        thetaPerDay_        = moreResults->thetaPerDay;
        strikeSensitivity_  = moreResults->strikeSensitivity;
        itmCashProbability_ = moreResults->itmCashProbability;
    }

}

