/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

#include <ql/pricingengines/basket/operatorsplittingspreadengine.hpp>

#include <ql/math/functional.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    OperatorSplittingSpreadEngine::OperatorSplittingSpreadEngine(
        ext::shared_ptr<BlackProcess> process1,
        ext::shared_ptr<BlackProcess> process2,
        Real correlation)
    : SpreadBlackScholesVanillaEngine(process1, process2, correlation) {
    }

    Real OperatorSplittingSpreadEngine::calculate(
        Real k, Option::Type optionType,
        Real variance1, Real variance2, DiscountFactor df) const {

        const Real vol1 = std::sqrt(variance1);
        const Real vol2 = std::sqrt(variance2);
        const Real sig2 = vol2*f2_/(f2_+k);
        const Real sig_m = std::sqrt(variance1 +sig2*(sig2 - 2*rho_*vol1));

        const Real d1 = (std::log(f1_) - std::log(f2_ + k))/sig_m + 0.5*sig_m;
        const Real d2 = d1 - sig_m;

        const CumulativeNormalDistribution N;
        const Real kirkNPV = df*(f1_*N(d1) - (f2_ + k)*N(d2));

        const Real v = (rho_*vol1 - sig2)*vol2/(sig_m*sig_m);
        const Real approx = kirkNPV
            - 0.5 * sig2*sig2 * k * df * NormalDistribution()(d2) * v
              *( d2*(1 - rho_*vol1/sig2)
                 - 0.5*sig_m * v * k / (f2_+k)
                   * ( d1*d2 + (1-rho_*rho_)*squared(vol1/(rho_*vol1-sig2))));

        if (optionType == Option::Call)
            return approx;
        else
            return approx - df*(f1_-f2_-k);
    }
}


