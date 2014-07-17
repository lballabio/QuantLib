/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009, 2014 Jose Aparicio

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

#ifndef quantlib_inhomogenous_pool_default_model_hpp
#define quantlib_inhomogenous_pool_default_model_hpp

#include <ql/experimental/credit/lossdistribution.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/constantlosslatentmodel.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>


namespace QuantLib {

    //-------------------------------------------------------------------------
    //! Default loss distribution convolution for finite homogeneous pool
    /* A note on the number of buckets: As it is now the code goes splitting
    losses into buckets from loses equal to zero to losses up to the value of
    the underlying basket. This is in view of a stochastic loss given default
    but in a constant LGD situation this is a waste and it is more efficient to
    go up to the attainable losses.
    */
    // Identical, except for the distribution type, to the homogeneous 
    //   case. Merge them
    //
    // Intended to replace HomogeneousPoolCDOEngine in syntheticcdoengines.hpp
    class InhomogeneousPoolLossModel : public DefaultLossModel {
    private:
        void resetModel() /*const*/;
    public:
        InhomogeneousPoolLossModel(
      ////      const boost::shared_ptr<GaussianConstantLossLM>& copula,
            const boost::shared_ptr<GaussianDefProbLM>& copula,
            const std::vector<Real>& recoveries,
            Size nBuckets,
            Real max = 5.,
            Real min = -5.,
            Real nSteps = 50)
        : copula_(copula), 
          nBuckets_(nBuckets), 
          recoveries_(recoveries),
          max_(max), min_(min), nSteps_(nSteps), delta_((max - min)/nSteps)
        { 
            QL_REQUIRE(copula->numFactors() == 1, "Not implemented for multifactor");
        }
// Write another constructor sending the LM factors and recoveries.

        ////void initialize(const Basket& basket) {
        ///void setupBasket(const boost::shared_ptr<Basket>& basket);

    protected:
        //////////////////////////////Real recoveryValueImpl(const Date& defaultDate, Size iName,
        //////////////////////////////    const std::vector<DefaultProbKey>& defKeys = 
        //////////////////////////////        std::vector<DefaultProbKey>()) const {
        //////////////////////////////        return recoveries_[iName];
        //////////////////////////////}
        Distribution lossDistrib(const Date& d) const;
    public:
        // TO DO: write a multifactor version 
        Real expectedTrancheLoss(const Date& d) const {
            return lossDistrib(d).trancheExpectedValue(attach_ * notional_, 
                detach_ * notional_);
        }
        Real percentile(const Date& d, Real percentile) const {
            Real portfLoss = lossDistrib(d).confidenceLevel(percentile);
            return std::min(std::max(portfLoss - attachAmount_, 0.), 
                detachAmount_ - attachAmount_);
        }
        Real expectedShortfall(const Date& d, Probability percentile) const {
            Distribution dist = lossDistrib(d);
            dist.tranche(attachAmount_, detachAmount_);
            return dist.expectedShortfall(percentile);
        }
    protected:
        const boost::shared_ptr<GaussianDefProbLM> copula_;
 ////////////////////////       const boost::shared_ptr<GaussianConstantLossLM> copula_;
        Size nBuckets_;
        const std::vector<Real> recoveries_;
 ///////////////////////////       boost::shared_ptr<Basket> basket_;
        mutable Real attach_, detach_, notional_, attachAmount_, detachAmount_;
        mutable std::vector<Real> notionals_;
    private:
        // integration:
        //  \todo move integration to latent model types when moving to a multifactor version
        const Real max_;// redundant?
        const Real min_;
        const Real nSteps_;
        const Real delta_; 
    };
///// ADD TH EMISSING MEMBERS FROM LOSS DISTRIB!!!!!!!!
}

#endif
