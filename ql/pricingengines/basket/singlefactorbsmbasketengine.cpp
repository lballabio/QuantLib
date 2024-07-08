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

/*! \file singlefactorbsmnasketengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/pricingengines/basket/vectorbsmprocessextractor.hpp>
#include <ql/pricingengines/basket/singlefactorbsmbasketengine.hpp>

namespace QuantLib {
    SingleFactorBsmBasketEngine::SingleFactorBsmBasketEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > p)
      : n_(p.size()), processes_(std::move(p)) {
        for (const auto& process: processes_)
            registerWith(process);
    }

    Real SingleFactorBsmBasketEngine::rootSumExponentials(
        const Array& a, const Array& sig, Real K) {

        const Array attr = a*sig;
        QL_REQUIRE(
            std::all_of(
                attr.begin(), attr.end(),
                [](Real x) -> bool { return x >= 0.0; }
            ),
            "a*sig should not be negative"
        );

        const bool logProb =
            std::all_of(
                a.begin(), a.end(),
                [](Real x) -> bool { return x > 0;}
        );

        QL_REQUIRE(K > 0 || !logProb,
            "non-positive strikes only allowed for spread options");

        // linear approximation
        const Real denom = std::accumulate(attr.begin(), attr.end(), 0.0);
        const Real xInit = (std::abs(denom) > 1000*QL_EPSILON)
            ? std::max(5.0, std::min(-5.0,
                    (K - std::accumulate(a.begin(), a.end(), 0.0))/denom)
              )
            : 0.0;

        const auto sumExp = [&a, &sig, K](Real x) -> Real {
            Real s = 0.0;
            for (Size i=0; i < a.size(); ++i)
                s += a[i]*std::exp(sig[i]*x);
            return s - K;
        };

        return xInit;
    }

    void SingleFactorBsmBasketEngine::calculate() const {
        const ext::shared_ptr<AverageBasketPayoff> avgPayoff =
            ext::dynamic_pointer_cast<AverageBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(avgPayoff, "average basket payoff expected");

        // sort assets by their weight
        const Array weights = avgPayoff->weights();
        QL_REQUIRE(n_ == weights.size(),
             "wrong number of weights arguments in payoff");

        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European exercise");
        const Date maturityDate = exercise->lastDate();

        const detail::VectorBsmProcessExtractor pExtractor(processes_);
        const Array s = pExtractor.getSpot();
        const Array dq = pExtractor.getDividendYield(maturityDate);
        const Array v = pExtractor.getBlackVariance(maturityDate);
        const DiscountFactor dr0 = pExtractor.getInterestRate(maturityDate);



    }
}

