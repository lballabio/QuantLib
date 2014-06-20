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

#include <ql/experimental/credit/inhomogeneouspooldef.hpp>

namespace QuantLib {

    void InhomogeneousPoolLossModel::setupBasket(
        const boost::shared_ptr<Basket>& basket) 
    {
        basket_ = basket;
        // need to be capped now since the limit amounts might be over the 
        //  remaining notional (think amortizing)
        attach_ = std::min(basket->remainingAttachmentAmount() / 
            basket->remainingNotional(), 1.);
        detach_ = std::min(basket->remainingDetachmentAmount() / 
            basket->remainingNotional(), 1.);
        notional_ = basket->remainingNotional();
        notionals_ = basket->remainingNotionals();
        attachAmount_ = basket->remainingAttachmentAmount();
        detachAmount_ = basket->remainingDetachmentAmount();
    }

    Distribution InhomogeneousPoolLossModel::lossDistrib(const Date& d) const {
        LossDistBucketing bucktLDistBuff(nBuckets_, notional_);
        std::vector<Real> lgd;// switch to a mutable cache member
        std::transform(recoveries_.begin(), recoveries_.end(), 
            std::back_inserter(lgd), std::bind1st(std::minus<Real>(), 1.));
        std::transform(lgd.begin(), lgd.end(), notionals_.begin(), 
            lgd.begin(), std::multiplies<Real>());
        std::vector<Real> prob = basket_->remainingProbabilities(d);

        // integrate locally (1 factor). 
        // use explicitly a 1D latent model object? 
        Distribution dist(nBuckets_, 0.0, notional_);
        std::vector<Real> mkft(1, min_ + delta_ /2.);
        for (Size i = 0; i < nSteps_; i++) {
            std::vector<Real> conditionalProbs;
            for(Size iName=0; iName<notionals_.size(); iName++)
                conditionalProbs.push_back(
                copula_->conditionalDefaultProbability(prob[iName], iName, 
                    mkft));
            Distribution d = bucktLDistBuff(lgd, conditionalProbs);
            Real densitydm = delta_ * copula_->density(mkft);
            // also, instead of calling the static method it could be wrapped 
            // through an inlined call in the latent model
            for (Size j = 0; j < nBuckets_; j++)
                dist.addDensity(j, d.density(j) * densitydm);
            mkft[0] += delta_;
        }
        return dist;
    }

}
