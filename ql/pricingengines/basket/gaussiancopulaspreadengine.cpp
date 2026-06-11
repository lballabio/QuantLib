/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yassine Idyiahia

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/pricingengines/basket/gaussiancopulaspreadengine.hpp>
#include <ql/exercise.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/methods/finitedifferences/utilities/smilesectionrndcalculator.hpp>
#include <ql/termstructures/volatility/atmsmilesection.hpp>
#include <utility>
#include <algorithm>
#include <cmath>

namespace QuantLib {

    GaussianCopulaSpreadEngine::GaussianCopulaSpreadEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Real correlation,
        Size nPoints)
    : process1_(std::move(process1)),
      process2_(std::move(process2)),
      rho_(correlation),
      nPoints_(nPoints) {
        QL_REQUIRE(correlation >= -1.0 && correlation <= 1.0,
                   "correlation must be in [-1, 1], got " << correlation);
        QL_REQUIRE(process1_->riskFreeRate().currentLink() ==
                   process2_->riskFreeRate().currentLink(),
                   "process1 and process2 must share the risk-free term "
                   "structure (used for discounting the spread payoff)");
        registerWith(process1_);
        registerWith(process2_);
    }

    void GaussianCopulaSpreadEngine::calculate() const {
        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not a European exercise");

        const ext::shared_ptr<SpreadBasketPayoff> spreadPayoff =
            ext::dynamic_pointer_cast<SpreadBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(spreadPayoff, "spread payoff expected");

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(spreadPayoff->basePayoff());
        QL_REQUIRE(payoff, "non-plain payoff given");

        // 1. Forwards, discount, and smile sections.
        const Date maturityDate = exercise->lastDate();

        const Real fwd1 = process1_->stateVariable()->value()
            * process1_->dividendYield()->discount(maturityDate)
            / process1_->riskFreeRate()->discount(maturityDate);

        const Real fwd2 = process2_->stateVariable()->value()
            * process2_->dividendYield()->discount(maturityDate)
            / process2_->riskFreeRate()->discount(maturityDate);

        const DiscountFactor df = process1_->riskFreeRate()->discount(maturityDate);

        const Time T1 = process1_->blackVolatility()->timeFromReference(maturityDate);
        const Time T2 = process2_->blackVolatility()->timeFromReference(maturityDate);

        const auto smile1 = ext::make_shared<AtmSmileSection>(
            process1_->blackVolatility()->smileSection(T1), fwd1);
        const auto smile2 = ext::make_shared<AtmSmileSection>(
            process2_->blackVolatility()->smileSection(T2), fwd2);

        // 2. Risk-neutral marginals.
        SmileSectionRNDCalculator rnd1(smile1);
        SmileSectionRNDCalculator rnd2(smile2);

        // 3. Nested Gauss-Hermite quadrature.  GaussHermiteIntegration
        // approximates int f(x) dx, so to integrate against phi(z) we
        // substitute z = sqrt(2)*x and weight by (1/pi) exp(-x_i^2 - x_j^2).

        GaussHermiteIntegration gh(nPoints_);

        const Array& x = gh.x();
        const Array& w = gh.weights();

        const CumulativeNormalDistribution Phi;
        const Real rhoComp = std::sqrt(std::max(1.0 - rho_ * rho_, 0.0));
        const Real normFactor = 1.0 / M_PI;

        Real sum = 0.0;
        for (Size i = 0; i < gh.order(); ++i) {
            const Real z1 = M_SQRT2 * x[i];
            // At extreme GH nodes Phi(z) saturates to exactly 0 or 1;
            // nudge it inside the open interval that invcdf requires.
            const Real u1 = std::clamp(Phi(z1), Real(QL_EPSILON), Real(1.0 - QL_EPSILON));
            const Real S1 = std::exp(rnd1.invcdf(u1));
            const Real expX1sq = std::exp(-x[i] * x[i]);

            Real innerSum = 0.0;
            for (Size j = 0; j < gh.order(); ++j) {
                const Real z2perp = M_SQRT2 * x[j];
                const Real z2 = rho_ * z1 + rhoComp * z2perp;
                const Real u2 = std::clamp(Phi(z2), Real(QL_EPSILON), Real(1.0 - QL_EPSILON));
                const Real S2 = std::exp(rnd2.invcdf(u2));

                const Real payoffVal = (*payoff)(S1 - S2);

                const Real kernel = expX1sq * std::exp(-x[j] * x[j]);
                innerSum += w[j] * kernel * payoffVal;
            }
            sum += w[i] * innerSum;
        }

        results_.value = df * normFactor * sum;
    }

}
