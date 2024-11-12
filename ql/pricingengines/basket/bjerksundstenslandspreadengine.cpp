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

#include <ql/pricingengines/basket/bjerksundstenslandspreadengine.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    BjerksundStenslandSpreadEngine::BjerksundStenslandSpreadEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Real correlation)
    : SpreadBlackScholesVanillaEngine(process1, process2, correlation) {
    }

    Real BjerksundStenslandSpreadEngine::calculate(
        Real f1, Real f2, Real k, Option::Type optionType,
        Real variance1, Real variance2, DiscountFactor df) const {

        const Real cp = (optionType == Option::Call) ? 1 : -1;

        const Real a = f2 + k;
        const Real b = f2/a;

        const Real sigma1 = std::sqrt(variance1);
        const Real sigma2 = std::sqrt(variance2);

        const Real stdev = std::sqrt(
            variance1 + b*b*variance2 - 2*rho_*b*sigma1*sigma2);

        const Real lfa = std::log(f1/a);

        const Real d1 =
            (lfa + (0.5*variance1 + 0.5*b*b*variance2 - b*rho_*sigma1*sigma2))/stdev;
        const Real d2 =
            (lfa + (-0.5*variance1 + variance2*b*(0.5*b - 1) + rho_*sigma1*sigma2))/stdev;
        const Real d3 = (lfa + (-0.5*variance1 + 0.5*b*b*variance2))/stdev;

        const CumulativeNormalDistribution phi;
        return df*cp*(f1*phi(cp*d1) - f2*phi(cp*d2) - k*phi(cp*d3));
    }
}


