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

#ifndef quantlib_tcopula_policy_hpp
#define quantlib_tcopula_policy_hpp

#include <ql/errors.hpp>
#include <ql/experimental/math/convolvedstudentt.hpp>
#include <ql/functional.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <vector>

namespace QuantLib {

    /*! \brief Student-T Latent Model's copula policy.

    Describes the copula of a set of normalized Student-T independent random 
    factors to be fed into the latent variable model. 
    The latent model requires the independent variables to be of unit variance 
    so the policy expects the factors coefficients to be as usual and the T 
    variables to be normalized, the normalization is performed by the policy. 
    To normalize the random variables they are divided by the square root of 
    the variance of each T (\f$ \frac{\nu}{\nu-2}\f$)
    */
    class TCopulaPolicy {
    public:
        /*! Stores the parameters defining the factors random variable 
        T-distributions. As it is now the latent models are restricted to
        having the same distribution for all idiosyncratic factors, so only
        one parameter is needed for them.
        */
        typedef 
            struct { 
                std::vector<Integer> tOrders;
            } initTraits;

        /*! Delayed initialization of the distribution parameters and caches. 
        To be called by the latent model. */
        /* \todo 
        Explore other constructors, with different vector dimensions, defining
        simpler combinations (only one correlation, only one variable) might
        simplify memory.
        */
        explicit TCopulaPolicy(
            const std::vector<std::vector<Real> >& factorWeights = 
                std::vector<std::vector<Real> >(), 
            const initTraits& vals = initTraits());

        //! Number of independent random factors.
        Size numFactors() const {
            return latentVarsInverters_.size() + varianceFactors_.size() - 1;
        }

        //! returns a copy of the initialization arguments
        //... better to have a cache?
        initTraits getInitTraits() const {
            initTraits data;
            data.tOrders.resize(distributions_.size());
            for (Size i=0; i<distributions_.size(); ++i) {
                data.tOrders[i] = static_cast<Integer>(
                    distributions_[i].degrees_of_freedom());
            }
            return data;
        }
        const std::vector<Real>& varianceFactors() const {
            return varianceFactors_;
        }
        /*! Cumulative probability of a given latent variable.
            The iVariable parameter is the index of the requested variable.
        */
        Probability cumulativeY(Real val, Size iVariable) const {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(iVariable < latentVarsCumul_.size(), 
                "Latent variable index out of bounds.");
    #endif
            return latentVarsCumul_[iVariable](val);
        }
        //! Cumulative probability of the idiosyncratic factors (all the same)
        Probability cumulativeZ(Real z) const {
            return boost::math::cdf(distributions_.back(), z / 
                varianceFactors_.back());
        }
        /*! Probability density of a given realization of values of the systemic
          factors (remember they are independent).
          Intended to be used in numerical integration of an arbitrary function 
          depending on those values.
        */
        Probability density(const std::vector<Real>& m) const {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(m.size() == distributions_.size()-1, 
                "Incompatible sample and latent model sizes");
    #endif
            Real prodDensities = 1.;
            for(Size i=0; i<m.size(); i++) 
                prodDensities *= boost::math::pdf(distributions_[i], 
                    m[i] /varianceFactors_[i]) /varianceFactors_[i];
                 // accumulate lambda
            return prodDensities;
        }
        /*! Returns the inverse of the cumulative distribution of the (modelled) 
          latent variable (as indexed by iVariable). Involves the convolution
          of the factors' distributions.
        */
        Real inverseCumulativeY(Probability p, Size iVariable) const {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(iVariable < latentVarsCumul_.size(), 
                "Latent variable index out of bounds.");
    #endif
            return latentVarsInverters_[iVariable](p);
        }
        /*! Returns the inverse of the cumulative distribution of the 
        idiosincratic factor. The LM here is limited to all idiosincratic 
        factors following the same distribution.
        */
        Real inverseCumulativeZ(Probability p) const {
            return boost::math::quantile(distributions_.back(), p)
                * varianceFactors_.back();
        }
        /*! Returns the inverse of the cumulative distribution of the 
          systemic factor iFactor.
        */
        Real inverseCumulativeDensity(Probability p, Size iFactor) const {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(iFactor < distributions_.size()-1, 
                "Random factor variable index out of bounds.");
    #endif
            return boost::math::quantile(distributions_[iFactor], p)
                * varianceFactors_[iFactor];
        }
        //to use this (by default) version, the generator must be a uniform one.
        std::vector<Real> allFactorCumulInverter(const std::vector<Real>& probs) const;
    private:
        mutable std::vector<boost::math::students_t_distribution<Real> > distributions_;
        mutable std::vector<Real> varianceFactors_;
        mutable std::vector<CumulativeBehrensFisher> latentVarsCumul_;
        mutable std::vector<InverseCumulativeBehrensFisher> latentVarsInverters_;
    };

}

#endif
