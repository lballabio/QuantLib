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

#include <vector>
#include <numeric>
#include <algorithm>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <ql/utilities/disposable.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    // In general (not this one) a base class with a convolution of the copula 
    // and a tabulated inversion as default behaviours makes sense...?
    /*! Gaussian Latent Model's copula policy. Its simplicity is a result of 
      the convolution stability of the Gaussian distribution.
    */
    struct GaussianCopulaPolicy {// make it come from an interface?

        typedef int initTraits;//int or anything else, it is not used<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< USE BOOST OPTIONAL so it can actually be called...void?

       /* static void */GaussianCopulaPolicy(const initTraits&)
           : density_(), cumulative_(){}/////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<default value in argument....

        /*! Cumulative probability of the indexed latent variable 
            @param iVariable The index of the latent variable requested.
        */
        /*static*/ Probability cumulativeY(Real val, Size iVariable) const {
            return cumulative_(val);
        }
        //! Cumulative probability of the idiosyncratic factors (all the same)
        /*static*/ Probability cumulativeZ(Real z) const {
            return cumulative_(z);
        }
        /*! Probability density of a given realization of values of the systemic
          factors (remember they are independent). In the normal case, since 
          they all follow the same law it is just a trivial product of the same 
          density. 
          Intended to be used in numerical integration of an arbitrary function 
          depending on those values.
        */
        /*static*/ Probability density(const std::vector<Real>& m) const {
            return std::accumulate(m.begin(), m.end(), 1., 
                boost::lambda::bind(std::multiplies<Real>(), boost::lambda::_1,
                    boost::lambda::bind(density_, boost::lambda::_2)));
        }
        /*! Returns the inverse of the cumulative distribution of the (modelled) 
          latent variable (as indexed by iVariable). The normal stability avoids
          the convolution of the factors' distributions
        */
        /*static*/ Real inverseCumulativeY(Probability p, Size iVariable) const {
            return InverseCumulativeNormal::standard_value(p);
        }
        /*! Returns the inverse of the cumulative distribution of the 
        idiosyncratic factor (identically distributed for all latent variables)
        */
        /*static*/ Real inverseCumulativeZ(Probability p) const {
            return InverseCumulativeNormal::standard_value(p);
        }
        /*! Returns the inverse of the cumulative distribution of the 
          systemic factor iFactor.
        */
        /*static*/ Real inverseCumulativeDensity(Probability p, int iFactor) const {
            return InverseCumulativeNormal::standard_value(p);
        }
        //to use this (by default) version, the generator must be a uniform one.
        /*static*/ Disposable<std::vector<Real> > 
            allFactorCumulInverter(const std::vector<Real>& probs) const {
            std::vector<Real> result;
            result.resize(probs.size());
            std::transform(probs.begin(), probs.end(), result.begin(), 
                boost::lambda::bind(&InverseCumulativeNormal::standard_value, 
                    boost::lambda::_1));
            return result;
        }
    private:
        /*static*/ const NormalDistribution density_;
        /*static*/ const CumulativeNormalDistribution cumulative_;
    };

}

#endif
