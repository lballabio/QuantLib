/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Cristina Duminuco

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

#include <ql/MarketModels/CurveStates/coterminalswapcurvestate.hpp>

namespace QuantLib {

    CoterminalSwapCurveState::CoterminalSwapCurveState(const std::vector<Time>& rateTimes)
    : NewCurveState(rateTimes),
      first_(nRates_), firstCotSwap_(nRates_),
      forwardRates_(nRates_), discRatios_(nRates_+1, 1.0),
      cotSwapRates_(nRates_), cotAnnuities_(nRates_) {}

      void CoterminalSwapCurveState::setOnCoterminalSwapRates(
                                        const std::vector<Rate>& swapRates,
                                        Size firstValidIndex) {
            QL_REQUIRE(swapRates.size()==nRates_,
                       "rates mismatch: " <<
                       nRates_ << " required, " <<
                       swapRates.size() << " provided");
            QL_REQUIRE(firstValidIndex<nRates_,
                       "first valid index must be less than " <<
                       nRates_ << ": " <<
                       firstValidIndex << " not allowed");

        // coterminal swap rates
        first_ = firstValidIndex;
        std::copy(swapRates.begin()+first_, swapRates.end(),
                  cotSwapRates_.begin()+first_);
        
        // discount ratios and coterminal annuities
        // reference discount bond is the last one P(n)=d[nRates_+1]: P(n)/P(n) = 1
        // discRatios_[nRates_+1] = 1.0; by construction
        cotAnnuities_[nRates_] = taus_[nRates_];
        // j < n
        for (Integer i=nRates_; i>0; --i) { // controllare estremi loop
            discRatios_[i] = 1.0 + cotSwapRates_[i] * cotAnnuities_[i];
            cotAnnuities_[i-1] = cotAnnuities_[i] + taus_[i] * discRatios_[i];            
        }
        discRatios_[0] = 1.0 + cotSwapRates_[0] * cotAnnuities_[1]; 
    }

    const std::vector<Rate>& CoterminalSwapCurveState::forwardRates() const {
        // controls
        return forwardRates_;
    }

    const std::vector<Rate>&
    CoterminalSwapCurveState::coterminalSwapAnnuities() const {
        // controls
        return cotAnnuities_;
    }

    const std::vector<Rate>& CoterminalSwapCurveState::coterminalSwapRates() const {
        // todo controls
        return cotSwapRates_;
    }

    Real CoterminalSwapCurveState::discountRatio(Size i, Size j) const {
        Size iMin = std::min(i, j);
        QL_REQUIRE(iMin>=first_, "index too low");
        QL_REQUIRE(std::max(i, j)<=nRates_, "index too high");
        return discRatios_[i]/discRatios_[j];
    }

    Rate CoterminalSwapCurveState::coterminalSwapAnnuity(Size i) const {
        // controls
        return cotAnnuities_[i];
    }

    Rate CoterminalSwapCurveState::coterminalSwapRate(Size i) const {
        // controls
        return cotSwapRates_[i];
    }

    // Computation of fwd rates
    // from coterminal swap rates and annuities
    void CoterminalSwapCurveState::computeForwardRate(Size i) const {
        // controls    
        forwardRates_[i] = (discountRatio(i,i+i)-1.0)/taus_[i-1];
    }

}
