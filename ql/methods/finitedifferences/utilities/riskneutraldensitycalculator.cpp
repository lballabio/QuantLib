/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/methods/finitedifferences/utilities/riskneutraldensitycalculator.hpp>
#include <ql/functional.hpp>

namespace QuantLib {
    RiskNeutralDensityCalculator::InvCDFHelper::InvCDFHelper(
        const RiskNeutralDensityCalculator* calculator,
        Real guess, Real accuracy, Size maxEvaluations,
        Real stepSize)
    : calculator_(calculator),
      guess_(guess),
      accuracy_(accuracy),
      maxEvaluations_(maxEvaluations),
      stepSize_(stepSize) { }

    Real RiskNeutralDensityCalculator::InvCDFHelper::inverseCDF(Real p, Time t)
    const {
        using namespace ext::placeholders;

        const ext::function<Real(Real)> cdf
            = ext::bind(&RiskNeutralDensityCalculator::cdf,
                          calculator_, _1, t);

        Brent solver;
        solver.setMaxEvaluations(maxEvaluations_);
        return solver.solve(compose(subtract<Real>(p), cdf),
                            accuracy_, guess_, stepSize_);
    }
}
