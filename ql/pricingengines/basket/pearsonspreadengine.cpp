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

#include <ql/pricingengines/basket/pearsonspreadengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <utility>

namespace QuantLib {

    PearsonSpreadEngine::PearsonSpreadEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Real correlation,
        Real integrationTolerance,
        Size maxIntegrationIterations,
        Real nStd)
    : SpreadBlackScholesVanillaEngine(
          std::move(process1), std::move(process2), correlation),
      integrationTolerance_(integrationTolerance),
      maxIntegrationIterations_(maxIntegrationIterations),
      nStd_(nStd) {
    }

    Real PearsonSpreadEngine::calculate(
        Real f1, Real f2, Real strike, Option::Type optionType,
        Real variance1, Real variance2, DiscountFactor df) const {

        const Real sigma1 = std::sqrt(variance1);
        const Real sigma2 = std::sqrt(variance2);

        // Under the forward measure, the joint dynamics are:
        //   ln F1 ~ N(mu1, sigma1^2),  ln F2 ~ N(mu2, sigma2^2)
        //   with correlation rho_
        //
        // Condition on z = standard normal driving F2:
        //   F2(z) = f2 * exp(-0.5*sigma2^2 + sigma2*z)
        //
        // Conditional on z, F1 is log-normal with:
        //   conditional mean of ln F1:  mu1_cond = ln(f1) - 0.5*sigma1^2 + rho_*sigma1*z
        //   conditional variance:       sigma1_cond^2 = sigma1^2*(1 - rho_^2)
        //
        // The conditional spread option value is a Black call/put:
        //   BS(F1_cond, K + F2(z), sigma1_cond)

        const Real sigma1Cond = sigma1 * std::sqrt(std::max(1.0 - rho_ * rho_, 0.0));

        const NormalDistribution phi;

        // Integrate over the standard normal variable z
        // that drives F2. Truncate at +/- 8 standard deviations.
        auto integrand = [&](Real z) -> Real {
            const Real f2z = f2 * std::exp(-0.5 * variance2 + sigma2 * z);
            const Real effectiveStrike = f2z + strike;

            if (effectiveStrike <= 0.0)
                return phi(z) * std::max(0.0, f1 * std::exp(rho_ * sigma1 * z
                           - 0.5 * rho_ * rho_ * variance1) - effectiveStrike);

            const Real f1Cond = f1 * std::exp(rho_ * sigma1 * z
                                              - 0.5 * rho_ * rho_ * variance1);

            BlackCalculator black(
                ext::make_shared<PlainVanillaPayoff>(optionType, effectiveStrike),
                f1Cond, sigma1Cond, 1.0);

            return phi(z) * black.value();
        };

        GaussLobattoIntegral integrator(maxIntegrationIterations_,
                                       integrationTolerance_);
        const Real undiscounted = integrator(integrand, -nStd_, nStd_);

        return df * undiscounted;
    }
}
