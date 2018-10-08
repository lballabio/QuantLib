/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#include <ql/experimental/math/tcopulapolicy.hpp>
#include <ql/bind.hpp>
#include <numeric>
#include <algorithm>

namespace QuantLib {

    TCopulaPolicy::TCopulaPolicy(
        const std::vector<std::vector<Real> >& factorWeights, 
        const initTraits& vals)
    {
        for(Size iFactor=0; iFactor<vals.tOrders.size(); iFactor++) {
            // require no T is of order 2 (finite variance)
            QL_REQUIRE(vals.tOrders[iFactor] > 2, 
                "Non finite variance T in latent model.");

            distributions_.push_back(boost::math::students_t_distribution<>(
                vals.tOrders[iFactor]));
            // inverses T variaces used in normalization of the random factors
            // For low values of the T order this number is very close to zero 
            // and it enters the expresions dividing them, which introduces 
            // numerical errors.
            varianceFactors_.push_back(std::sqrt(
                (vals.tOrders[iFactor]-2.)/vals.tOrders[iFactor]));
        }

        for(Size iLVar=0; iLVar<factorWeights.size(); iLVar++) {
            // This ensures the latent model is 'canonical'
            QL_REQUIRE(vals.tOrders.size() == factorWeights[iLVar].size()+1, 
                // num factors plus one
                "Incompatible number of T functions and number of factors."); 

            Real factorsNorm = std::inner_product(factorWeights[iLVar].begin(), 
                factorWeights[iLVar].end(), factorWeights[iLVar].begin(), 0.);
            QL_REQUIRE(factorsNorm < 1., 
                "Non normal random factor combination.");
            Real idiosyncFctr = std::sqrt(1.-factorsNorm);

            // linear comb factors ajusted for the variance renormalization:
            std::vector<Real> normFactorWeights;
            for(Size iFactor=0; iFactor<factorWeights[iLVar].size(); iFactor++)
                normFactorWeights.push_back(factorWeights[iLVar][iFactor] * 
                    varianceFactors_[iFactor]);
            // idiosincratic term, all Z factors are assumed identical.
            normFactorWeights.push_back(idiosyncFctr * varianceFactors_.back());
            latentVarsCumul_.push_back( 
                CumulativeBehrensFisher(vals.tOrders, normFactorWeights));
            latentVarsInverters_.push_back(
                InverseCumulativeBehrensFisher(vals.tOrders, 
                    normFactorWeights));
        }
    }

    Disposable<std::vector<Real> > 
    TCopulaPolicy::allFactorCumulInverter(
        const std::vector<Real>& probs) const 
    {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(probs.size()-latentVarsCumul_.size() 
            == distributions_.size()-1, 
            "Incompatible sample and latent model sizes");
    #endif

        using namespace ext::placeholders;

        std::vector<Real> result(probs.size());
        Size indexSystemic = 0;
        std::transform(probs.begin(), probs.begin() + varianceFactors_.size()-1,
            result.begin(), 
            ext::bind(&TCopulaPolicy::inverseCumulativeDensity, 
                                this, _1, indexSystemic++));
        std::transform(probs.begin() + varianceFactors_.size()-1, probs.end(),
            result.begin()+ varianceFactors_.size()-1,
            ext::bind(&TCopulaPolicy::inverseCumulativeZ, this, _1));
        return result;
    }

}
