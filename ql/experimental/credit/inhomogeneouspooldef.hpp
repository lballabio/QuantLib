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

// Intended to replace InhomogeneousPoolCDOEngine in syntheticcdoengines.hpp

namespace QuantLib {

    //-------------------------------------------------------------------------
    //! Default loss distribution convolution for finite non homogeneous pool
    /* A note on the number of buckets: As it is now the code goes splitting
    losses into buckets from loses equal to zero to losses up to the value of
    the underlying basket. This is in view of a stochastic loss given default
    but in a constant LGD situation this is a waste and it is more efficient to
    go up to the attainable losses.
    \todo Extend to the multifactor case for a generic LM
    \todo Many common code with the homogeneous version, both classes perform
    the same work on different loss distribution types, merge and send the 
    distribution object?
    */
    template<class copulaPolicy>
    class InhomogeneousPoolLossModel : public DefaultLossModel {
    private:
      void resetModel() override;

    public:
        // allow base correlations:
        typedef copulaPolicy copulaType;

        InhomogeneousPoolLossModel(
        // restricted to non random recoveries, but it could be possible.
            const ext::shared_ptr<ConstantLossLatentmodel<copulaPolicy> >& 
                copula,
            Size nBuckets,
            Real max = 5.,
            Real min = -5.,
            Real nSteps = 50)
        : copula_(copula), 
          nBuckets_(nBuckets), 
          max_(max), min_(min), nSteps_(nSteps), delta_((max - min)/nSteps)
        { 
            QL_REQUIRE(copula->numFactors() == 1, 
                "Inhomogeneous model not implemented for multifactor");
        }
    // Write another constructor sending the LM factors and recoveries.
    protected:
        Distribution lossDistrib(const Date& d) const;
    public:
      Real expectedTrancheLoss(const Date& d) const override {
          return lossDistrib(d).cumulativeExcessProbability(attachAmount_, detachAmount_);
          // This one if the distribution is over the whole loss structure:
          // but it becomes very expensive
          /*
          return lossDistrib(d).trancheExpectedValue(
              attachAmount_, detachAmount_);
          */
      }
      Real percentile(const Date& d, Real percentile) const override {
          Real portfLoss = lossDistrib(d).confidenceLevel(percentile);
          return std::min(std::max(portfLoss - attachAmount_, 0.), detachAmount_ - attachAmount_);
      }
      Real expectedShortfall(const Date& d, Probability percentile) const override {
          Distribution dist = lossDistrib(d);
          dist.tranche(attachAmount_, detachAmount_);
          return dist.expectedShortfall(percentile);
      }

    protected:
        const ext::shared_ptr<ConstantLossLatentmodel<copulaPolicy> > copula_;
        Size nBuckets_;
        mutable Real attach_, detach_, notional_, attachAmount_, detachAmount_;
        mutable std::vector<Real> notionals_;
    private:
        // integration:
        //  \todo move integration to latent model types when moving to a 
        //  multifactor version
        const Real max_;// redundant?
        const Real min_;
        const Real nSteps_;
        const Real delta_; 
    };
    // \todo Add other loss distribution statistics
    typedef InhomogeneousPoolLossModel<GaussianCopulaPolicy> 
        IHGaussPoolLossModel;
    typedef InhomogeneousPoolLossModel<TCopulaPolicy> IHStudentPoolLossModel;

    //-----------------------------------------------------------------------

    template<class CP>
    void InhomogeneousPoolLossModel<CP>::resetModel()
    {
        // need to be capped now since the limit amounts might be over the 
        //  remaining notional (think amortizing)
        attach_ = std::min(basket_->remainingAttachmentAmount() / 
            basket_->remainingNotional(), 1.);
        detach_ = std::min(basket_->remainingDetachmentAmount() / 
            basket_->remainingNotional(), 1.);
        notional_ = basket_->remainingNotional();
        notionals_ = basket_->remainingNotionals();
        attachAmount_ = basket_->remainingAttachmentAmount();
        detachAmount_ = basket_->remainingDetachmentAmount();

        copula_->resetBasket(basket_.currentLink());
    }

    template<class CP>
    Distribution InhomogeneousPoolLossModel<CP>::lossDistrib(
        const Date& d) const 
    {
        LossDistBucketing bucktLDistBuff(nBuckets_, detachAmount_);

        std::vector<Real> lgd;// switch to a mutable cache member
        std::vector<Real> recoveries = copula_->recoveries();
        std::transform(recoveries.begin(), recoveries.end(), 
                       std::back_inserter(lgd),
                       [](Real x) -> Real { return 1.0-x; });
        std::transform(lgd.begin(), lgd.end(), notionals_.begin(), 
                       lgd.begin(), std::multiplies<>());
        std::vector<Real> prob = basket_->remainingProbabilities(d);
        for(Size iName=0; iName<prob.size(); iName++)
            prob[iName] = copula_->inverseCumulativeY(prob[iName], iName);

        // integrate locally (1 factor). 
        // use explicitly a 1D latent model object? 
        // \todo Use a library integrator here and in the homogeneous case.
        Distribution dist(nBuckets_, 0.0, 
            detachAmount_);
            //notional_);
        std::vector<Real> mkft(1, min_ + delta_ /2.);
        for (Size i = 0; i < nSteps_; i++) {
            std::vector<Real> conditionalProbs;
            for(Size iName=0; iName<notionals_.size(); iName++)
                conditionalProbs.push_back(
                copula_->conditionalDefaultProbabilityInvP(prob[iName], iName, 
                    mkft));
            Distribution bld = bucktLDistBuff(lgd, conditionalProbs);
            Real densitydm = delta_ * copula_->density(mkft);
            // also, instead of calling the static method it could be wrapped 
            // through an inlined call in the latent model
            for (Size j = 0; j < nBuckets_; j++)
                dist.addDensity(j, bld.density(j) * densitydm);
            mkft[0] += delta_;
        }
        return dist;
    }


}

#endif
