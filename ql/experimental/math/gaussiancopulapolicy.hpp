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

#ifndef quantlib_gaussian_copula_policy_hpp
#define quantlib_gaussian_copula_policy_hpp

#include <ql/utilities/disposable.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <vector>
#include <numeric>
#include <algorithm>

namespace QuantLib {

    /*! Gaussian Latent Model's copula policy. Its simplicity is a result of 
      the convolution stability of the Gaussian distribution.
    */
    /* This is the only case that would have allowed the policy to be static, 
    but other copulas will need parameters and initialization.*/
    struct GaussianCopulaPolicy {

        typedef int initTraits;

        explicit GaussianCopulaPolicy(
            const std::vector<std::vector<Real> >& factorWeights = 
                std::vector<std::vector<Real> >(), 
            const initTraits& dummy = int())
        : numFactors_(factorWeights.size() + factorWeights[0].size())
        {
            /* check factors in LM are normalized. */
            for (const auto& factorWeight : factorWeights) {
                Real factorsNorm = std::inner_product(factorWeight.begin(), factorWeight.end(),
                                                      factorWeight.begin(), 0.);
                QL_REQUIRE(factorsNorm < 1., 
                    "Non normal random factor combination.");
            }
            /* check factor matrix is squared .......... */
        }

        /*! Number of independent random factors. 
        This is the only methos that ould stop the class from being static, it
        is needed for the MC generator construction.
        */
        Size numFactors() const {
            return numFactors_;
        }

        //! returns a copy of the initialization arguments
        initTraits getInitTraits() const {
            return initTraits();
        }

        /*! Cumulative probability of a given latent variable 
            The iVariable parameter is the index of the requested variable.
        */
        Probability cumulativeY(Real val, Size iVariable) const {
            return cumulative_(val);
        }
        //! Cumulative probability of the idiosyncratic factors (all the same)
        Probability cumulativeZ(Real z) const {
            return cumulative_(z);
        }
        /*! Probability density of a given realization of values of the systemic
          factors (remember they are independent). In the normal case, since 
          they all follow the same law it is just a trivial product of the same 
          density. 
          Intended to be used in numerical integration of an arbitrary function 
          depending on those values.
        */
        Probability density(const std::vector<Real>& m) const {
            return std::accumulate(m.begin(), m.end(), Real(1.),
                                   [&](Real x, Real y){ return x*density_(y); });
        }
        /*! Returns the inverse of the cumulative distribution of the (modelled) 
          latent variable (as indexed by iVariable). The normal stability avoids
          the convolution of the factors' distributions
        */
        Real inverseCumulativeY(Probability p, Size iVariable) const {
            return InverseCumulativeNormal::standard_value(p);
        }
        /*! Returns the inverse of the cumulative distribution of the 
        idiosyncratic factor (identically distributed for all latent variables)
        */
        Real inverseCumulativeZ(Probability p) const {
            return InverseCumulativeNormal::standard_value(p);
        }
        /*! Returns the inverse of the cumulative distribution of the 
          systemic factor iFactor.
        */
        Real inverseCumulativeDensity(Probability p, Size iFactor) const {
            return InverseCumulativeNormal::standard_value(p);
        }
        //! 
        //to use this (by default) version, the generator must be a uniform one.
        Disposable<std::vector<Real> > 
            allFactorCumulInverter(const std::vector<Real>& probs) const {
            std::vector<Real> result;
            result.resize(probs.size());
            std::transform(probs.begin(), probs.end(), result.begin(),
                           [&](Real p){ return InverseCumulativeNormal::standard_value(p); });
            return result;
        }
    private:
        mutable Size numFactors_;
        // no op =
        static const NormalDistribution density_;
        static const CumulativeNormalDistribution cumulative_;
    };

}

#endif
