/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/curvestate.hpp>

namespace QuantLib {

    CurveState::CurveState(const std::vector<Time>& rateTimes)
    : rateTimes_(rateTimes.begin(), rateTimes.end()),
      rateTaus_(rateTimes_.size()-1),
      nRates_(rateTimes_.empty() ? 0 : rateTimes_.size()-1) {
        QL_REQUIRE(nRates_>0, "Rate times must contain at least two values");
        for (Size i=0; i<nRates_; ++i) {
            rateTaus_[i] = rateTimes_[i+1] - rateTimes_[i];
            QL_REQUIRE(rateTaus_[i]>0, "non increasing rate times");
        }
    }

    Rate CurveState::swapRate(Size begin,
                              Size end) const {

        QL_REQUIRE(end > begin, "empty range specified");
        QL_REQUIRE(end <= nRates_, "taus/end mismatch");

        Real sum = 0.0;
        for (Size i=begin; i<end; ++i)
            sum += rateTaus_[i]*discountRatio(i+1, nRates_);

        return (discountRatio(begin, nRates_)-discountRatio(end, nRates_))/sum;
    }

    void forwardsFromDiscountRatios(const Size firstValidIndex,
                                    const std::vector<DiscountFactor>& ds,
                                    const std::vector<Time>& taus,
                                    std::vector<Rate>& fwds) {
        QL_REQUIRE(taus.size()==fwds.size(),
                   "taus.size()!=fwds.size()");
        QL_REQUIRE(ds.size()==fwds.size()+1,
                   "ds.size()!=fwds.size()+1");

        for (Size i=firstValidIndex; i<fwds.size(); ++i)
            fwds[i] = (ds[i]-ds[i+1])/(ds[i+1]*taus[i]);
    }

    void coterminalFromDiscountRatios(
                    const Size firstValidIndex,
                    const std::vector<DiscountFactor>& discountFactors,
                    const std::vector<Time>& taus,
                    std::vector<Rate>& cotSwapRates,
                    std::vector<Real>& cotSwapAnnuities) 
    {
        Size nCotSwapRates = cotSwapRates.size();
        QL_REQUIRE(taus.size()==nCotSwapRates,
                   "taus.size()!=cotSwapRates.size()");
        QL_REQUIRE(cotSwapAnnuities.size()==nCotSwapRates,
                   "cotSwapAnnuities.size()!=cotSwapRates.size()");
        QL_REQUIRE(discountFactors.size()==nCotSwapRates+1,
                   "discountFactors.size()!=cotSwapRates.size()+1");

        cotSwapAnnuities[nCotSwapRates-1] = 
            taus[nCotSwapRates-1]*discountFactors[nCotSwapRates];
        cotSwapRates[nCotSwapRates-1] = 
            (discountFactors[nCotSwapRates-1]-discountFactors[nCotSwapRates])
                /cotSwapAnnuities[nCotSwapRates-1];

        for (Size i=nCotSwapRates-1; i>firstValidIndex; --i) {
            cotSwapAnnuities[i-1] = cotSwapAnnuities[i] + taus[i-1] * discountFactors[i];
            cotSwapRates[i-1] = 
                (discountFactors[i-1]-discountFactors[nCotSwapRates])
                /cotSwapAnnuities[i-1];
        }
    }


    void constantMaturityFromDiscountRatios(// Size i, // to be added later
                                            const Size spanningForwards,
                                            const Size firstValidIndex,
                                            const std::vector<DiscountFactor>& ds,
                                            const std::vector<Time>& taus,
                                            std::vector<Rate>& constMatSwapRates,
                                            std::vector<Real>& constMatSwapAnnuities) {
        Size nConstMatSwapRates = constMatSwapRates.size();
        QL_REQUIRE(taus.size()==nConstMatSwapRates,
                   "taus.size()!=nConstMatSwapRates");
        QL_REQUIRE(constMatSwapAnnuities.size()==nConstMatSwapRates,
                   "constMatSwapAnnuities.size()!=nConstMatSwapRates");
        QL_REQUIRE(ds.size()==nConstMatSwapRates+1,
                   "ds.size()!=nConstMatSwapRates+1");
        // compute the first cmsrate and cmsannuity
        constMatSwapAnnuities[firstValidIndex]=0.;
        Size lastIndex = std::min(firstValidIndex+spanningForwards,nConstMatSwapRates);
        for (Size i=firstValidIndex; i<lastIndex; ++i) {
            constMatSwapAnnuities[firstValidIndex]+= taus[i] * ds[i+1];
        }
        constMatSwapRates[firstValidIndex] =
            (ds[firstValidIndex]-ds[lastIndex])/
                constMatSwapAnnuities[firstValidIndex];
        Size oldLastIndex = lastIndex;

        // compute all the other cmas rates and cms annuities
        for (Size i=firstValidIndex+1; i<nConstMatSwapRates; ++i) {
            Size lastIndex = std::min(i+spanningForwards,nConstMatSwapRates);
            constMatSwapAnnuities[i] = constMatSwapAnnuities[i-1]
                                       - taus[i-1] * ds[i];
            if (lastIndex!=oldLastIndex)
               constMatSwapAnnuities[i] += taus[lastIndex-1] * ds[lastIndex];
            constMatSwapRates[i] = (ds[i]-ds[lastIndex])
                /constMatSwapAnnuities[i];
            oldLastIndex = lastIndex;
        }
    }

}
