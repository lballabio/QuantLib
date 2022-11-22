/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

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
#include <ql/models/marketmodels/pathwisediscounter.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <algorithm>

namespace QuantLib
{

MarketModelPathwiseDiscounter::MarketModelPathwiseDiscounter(Time paymentTime,
                              const std::vector<Time>& rateTimes)
{
    checkIncreasingTimes(rateTimes);

    numberRates_ = rateTimes.size()-1;
        before_ = std::lower_bound(rateTimes.begin(), rateTimes.end(),
                                   paymentTime) - rateTimes.begin();

        // handle the case where the payment is in the last
        // period or after the last period
        if (before_ > rateTimes.size()-2)
            before_ =  rateTimes.size()-2;

        beforeWeight_=1.0-(paymentTime-rateTimes[before_])/
            (rateTimes[before_+1]-rateTimes[before_]);

        postWeight_  = 1.0 - beforeWeight_;
        taus_.resize(numberRates_);

        for (Size i=0; i < numberRates_; ++i)
            taus_[i] = rateTimes[i+1] - rateTimes[i];

}

void MarketModelPathwiseDiscounter::getFactors(
            const Matrix& , // LIBORRates, for all steps
            const Matrix& Discounts, // P(t_0, t_j) for j=0,...n for each step
            Size currentStep,
            std::vector<Real>& factors) const
{
    Real preDF = Discounts[currentStep][before_];
    Real postDF = Discounts[currentStep][before_+1];

    for (Size i=before_+1; i<numberRates_; ++i)
        factors[i+1] =0.0;

    if (postWeight_==0.0)
    {
        factors[0] = preDF;

        for (Size i=0; i<before_; ++i)
            factors[i+1] = -preDF*taus_[i]*Discounts[currentStep][i+1]/Discounts[currentStep][i];

        factors[before_+1]=0.0;

        return;
    }

    Real df = preDF * std::pow(postDF/preDF, postWeight_);

    factors[0] = df;

    for (Size i=0; i<=before_; ++i)
       factors[i+1] = -df*taus_[i]*Discounts[currentStep][i+1]/Discounts[currentStep][i];

    factors[before_+1] *= postWeight_;
}
}
