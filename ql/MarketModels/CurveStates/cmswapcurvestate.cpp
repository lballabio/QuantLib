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

    void CMSwapCurveState::setOnCMSwapRates(
                    const std:vector<Rate>& cmSwapRates,
                    Size firstValidIndex = 0) {
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
            std::copy(begin+first_, end, cmSwaps_.begin()+first_);

            // discount ratios
            discRatios_[first_] = 1.0;
            for (Size i=first_; i<nRates_; ++i) {
            }

            // lazy evaluation of coterminal swap rates and annuities
            firstCotSwap_ = nRates_;
    }


    const std::vector<Rate>& CMSwapCurveState::forwardRates() const {
    }

    const std::vector<DiscountFactor>& CMSwapCurveState::discountRatios() const {
    }

    const std::vector<Real>& CMSwapCurveState::coterminalSwapAnnuities() const {
    }

    const std::vector<Rate>& CMSwapCurveState::coterminalSwapRates() const {
    }

    const std::vector<Real>& CMSwapCurveState::cmSwapAnnuities(Size spanningForwards) const {
    }

    const std::vector<Rate>& CMSwapCurveState::cmSwapRates(Size spanningForwards) const {
    }


    Rate CMSwapCurveState::forwardRate(Size i) const {
    }

    Rate CMSwapCurveState::coterminalSwapAnnuity(Size i) const {
    }

    Rate CMSwapCurveState::coterminalSwapRate(Size i) const {
    }

    Rate CMSwapCurveState::cmSwapAnnuity(Size i,
                             Size spanningForwards) const {
    }

    Rate CMSwapCurveState::cmSwapRate(Size i,
                          Size spanningForwards) const {
    }

}
