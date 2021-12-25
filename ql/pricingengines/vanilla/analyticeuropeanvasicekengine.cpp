/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

#include <ql/exercise.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanvasicekengine.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        Real g_k(Real t, Real kappa){
            return (1 - std::exp(- kappa * t )) / kappa;
        }

        class integrand_vasicek {
          private:
            const Real sigma_s_;
            const Real sigma_r_;
            const Real correlation_;
            const Real kappa_;
            const Real T_;
          public:
            integrand_vasicek(Real sigma_s, Real sigma_r, Real correlation, Real kappa, Real T)
            : sigma_s_(sigma_s), sigma_r_(sigma_r), correlation_(correlation), kappa_(kappa), T_(T){}
            Real operator()(Real u) const {
                Real g = g_k(T_ - u, kappa_);
                return (sigma_s_ * sigma_s_) + (2 * correlation_ * sigma_s_ * sigma_r_ * g) + (sigma_r_ * sigma_r_ * g * g);
            }
        };

    }

    AnalyticBlackVasicekEngine::AnalyticBlackVasicekEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> blackProcess,
        ext::shared_ptr<Vasicek> vasicekProcess,
        Real correlation)
    : blackProcess_(std::move(blackProcess)), vasicekProcess_(std::move(vasicekProcess)),
      simpsonIntegral_(new SimpsonIntegral(1e-5, 1000)), correlation_(correlation) {
        registerWith(blackProcess_);
        registerWith(vasicekProcess_);
    }

    void AnalyticBlackVasicekEngine::calculate() const {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        ext::shared_ptr<StrikedTypePayoff> payoff =
                ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        QL_REQUIRE(payoff, "non-striked payoff given");

        CumulativeNormalDistribution f;

        Real t = 0;
        Real T = blackProcess_->riskFreeRate()->dayCounter().yearFraction(blackProcess_->riskFreeRate().currentLink()->referenceDate(),arguments_.exercise->lastDate());
        Real kappa = vasicekProcess_->a();
        Real S_t = blackProcess_->x0();
        Real K = payoff->strike();
        Real sigma_s = blackProcess_->blackVolatility()->blackVol(t, K);
        Real sigma_r = vasicekProcess_->sigma();
        Real r_t = vasicekProcess_->r0();

        Real zcb = vasicekProcess_->discountBond(t, T, r_t);
        Real epsilon = payoff->optionType() == Option::Call ? 1 : -1;
        Real upsilon = (*simpsonIntegral_)(integrand_vasicek(sigma_s, sigma_r, correlation_, kappa, T), t, T);
        Real d_positive = (std::log((S_t / K) / zcb) + upsilon / 2) / std::sqrt(upsilon);
        Real d_negative = (std::log((S_t / K) / zcb) - upsilon / 2) / std::sqrt(upsilon);
        Real n_d1 = f(epsilon * d_positive);
        Real n_d2 = f(epsilon * d_negative);

        results_.value = epsilon * ((S_t * n_d1) - (zcb * K * n_d2));
    }

}

