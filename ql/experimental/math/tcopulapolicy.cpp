/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#include <ql/experimental/math/tcopulapolicy.hpp>
#include <numeric>
#include <algorithm>

namespace QuantLib {

    TCopulaPolicy::TCopulaPolicy(
        const std::vector<std::vector<Real> >& factorWeights, 
        const initTraits& vals)
    {
        for (int tOrder : vals.tOrders) {
            // require no T is of order 2 (finite variance)
            QL_REQUIRE(tOrder > 2, "Non finite variance T in latent model.");

            distributions_.emplace_back(tOrder);
            // inverses T variaces used in normalization of the random factors
            // For low values of the T order this number is very close to zero 
            // and it enters the expressions dividing them, which introduces 
            // numerical errors.
            varianceFactors_.push_back(std::sqrt((tOrder - 2.) / tOrder));
        }

        for (const auto& factorWeight : factorWeights) {
            // This ensures the latent model is 'canonical'
            QL_REQUIRE(vals.tOrders.size() == factorWeight.size() + 1,
                       // num factors plus one
                       "Incompatible number of T functions and number of factors.");

            Real factorsNorm = std::inner_product(factorWeight.begin(), factorWeight.end(),
                                                  factorWeight.begin(), Real(0.));
            QL_REQUIRE(factorsNorm < 1., 
                "Non normal random factor combination.");
            Real idiosyncFctr = std::sqrt(1.-factorsNorm);

            // linear comb factors ajusted for the variance renormalization:
            std::vector<Real> normFactorWeights;
            normFactorWeights.reserve(factorWeight.size());
            for (Size iFactor = 0; iFactor < factorWeight.size(); iFactor++)
                normFactorWeights.push_back(factorWeight[iFactor] * varianceFactors_[iFactor]);
            // idiosincratic term, all Z factors are assumed identical.
            normFactorWeights.push_back(idiosyncFctr * varianceFactors_.back());
            latentVarsCumul_.emplace_back(vals.tOrders, normFactorWeights);
            latentVarsInverters_.emplace_back(vals.tOrders, normFactorWeights);
        }
    }

    std::vector<Real> TCopulaPolicy::allFactorCumulInverter(
        const std::vector<Real>& probs) const 
    {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(probs.size()-latentVarsCumul_.size() 
            == distributions_.size()-1, 
            "Incompatible sample and latent model sizes");
    #endif

        std::vector<Real> result(probs.size());
        Size indexSystemic = 0;
        std::transform(probs.begin(), probs.begin() + varianceFactors_.size()-1,
                       result.begin(),
                       [&](Probability p) { return inverseCumulativeDensity(p, indexSystemic++); });
        std::transform(probs.begin() + varianceFactors_.size()-1, probs.end(),
                       result.begin()+ varianceFactors_.size()-1,
                       [&](Probability p) { return inverseCumulativeZ(p); });
        return result;
    }

}
