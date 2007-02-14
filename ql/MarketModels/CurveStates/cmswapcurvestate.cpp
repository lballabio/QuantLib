/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/MarketModels/CurveStates/cmswapcurvestate.hpp>

namespace QuantLib {

    CMSwapCurveState::CMSwapCurveState(const std::vector<Time>& rateTimes,
                    Size spanningForwards)
    : NewCurveState(rateTimes),
      spanningFwds_(spanningForwards),
      first_(nRates_),
      forwardRates_(nRates_),
      cotSwaps_(nRates_), cmSwaps_(nRates_),
      discRatios_(nRates_+1),
      cotAnnuities_(nRates_), cmSwapAnn_(nRates_) {
    }

    void CMSwapCurveState::setOnCMSwapRates(
                    const std::vector<Rate>& cmSwapRates,
                    Size firstValidIndex) {
        QL_REQUIRE(cmSwapRates.size()==nRates_,
                   "rates mismatch: " <<
                   nRates_ << " required, " <<
                   cmSwapRates.size() << " provided");
        QL_REQUIRE(firstValidIndex<nRates_,
                   "first valid index must be less than " <<
                   nRates_ << ": " <<
                   firstValidIndex << " not allowed");

            // forwards
            first_ = firstValidIndex;
            std::copy(cmSwapRates.begin()+first_, cmSwapRates.end(),
                     cmSwaps_.begin()+first_);

            // discount ratios
            discRatios_[first_] = 1.0;
            // to be removed ...
            for (Size i = nRates_ - spanningFwds_; i<= nRates_; ++i)
                discRatios_[i] = 1.0;
            for (Size i = nRates_ - spanningFwds_; i< nRates_; ++i)
                cmSwapAnn_[i] = Real(nRates_ - i);
            for (Integer i=static_cast<Integer>(nRates_)- spanningFwds_;
             i>=static_cast<Integer>(first_+1); --i) {
                 // formula 6.1 Joshi Liesch
               cmSwapAnn_[i-1]= cmSwapAnn_[i]-
                   forwardRates_[i+spanningFwds_]*taus_[i+spanningFwds_]+
                   forwardRates_[i]*taus_[i];
               discRatios_[i] = discRatios_[i + spanningFwds_] +
                    cmSwaps_[i]*cmSwapAnn_[i];
               forwardRates_[i]=(discRatios_[i]/discRatios_[i+1]-1.0) /
                    taus_[i];
            }

            // lazy evaluation of coterminal swap rates and annuities
    }


    const std::vector<Rate>& CMSwapCurveState::forwardRates() const {
        QL_FAIL("not implemented yet");
    }

    const std::vector<Real>& CMSwapCurveState::coterminalSwapAnnuities() const {
        QL_FAIL("not implemented yet");
    }

    const std::vector<Rate>& CMSwapCurveState::coterminalSwapRates() const {
        QL_FAIL("not implemented yet");
    }

    const std::vector<Real>& CMSwapCurveState::cmSwapAnnuities(Size spanningForwards) const {
        return cmSwapAnn_;
    }

    const std::vector<Rate>& CMSwapCurveState::cmSwapRates(Size spanningForwards) const {
        return cmSwaps_;
    }

    Rate CMSwapCurveState::forwardRate(Size i) const {
        QL_FAIL("not implemented yet");
    }

    Rate CMSwapCurveState::coterminalSwapAnnuity(Size i, Size numeraire) const {
        QL_FAIL("not implemented yet");
    }

    Rate CMSwapCurveState::coterminalSwapRate(Size i) const {
        QL_FAIL("not implemented yet");
    }

    const std::vector<DiscountFactor>& CMSwapCurveState::discountRatios() const {
        return discRatios_;
    }

    Real CMSwapCurveState::discountRatio(Size i, Size j) const {
        return discRatios_[i]/discRatios_[j];
    }
    
    Rate CMSwapCurveState::cmSwapAnnuity(Size i,
                                 Size spanningForwards,
                                 Size numeraire) const{
        QL_FAIL("not implemented yet");
    }
    
    Rate CMSwapCurveState::cmSwapRate(Size i,
                              Size spanningForwards) const{
        QL_FAIL("not implemented yet");
    }

}
