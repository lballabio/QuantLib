/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti
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

#include <ql/MarketModels/CurveStates/lmmcurvestate.hpp>

namespace QuantLib {

    LMMCurveState::LMMCurveState(const std::vector<Time>& rateTimes)
    : NewCurveState(rateTimes),
      first_(nRates_), firstCotSwap_(nRates_),
      forwardRates_(nRates_), discRatios_(nRates_+1, 1.0),
      cotSwaps_(nRates_), cotAnnuities_(nRates_) {}

    void LMMCurveState::setOnForwardRates(
                           const std::vector<Rate>& fwdRates,
                           Size firstValidIndex) {
        QL_REQUIRE(fwdRates.size()==nRates_,
                   "forward rates mismatch: " <<
                   nRates_ << " required, " <<
                   fwdRates.size() << " provided");
        QL_REQUIRE(firstValidIndex<nRates_,
                   "first valid index must be less than " <<
                   nRates_ << ": " <<
                   firstValidIndex << " not allowed");

        // forwards
        first_ = firstValidIndex;
        std::copy(fwdRates.begin()+first_, fwdRates.end(),
                  forwardRates_.begin()+first_);

        // discount ratios
        discRatios_[first_] = 1.0;
        for (Size i=first_; i<nRates_; ++i)
            discRatios_[i+1] = discRatios_[i] /
                                (1.0+forwardRates_[i]*taus_[i]);

        // lazy evaluation of coterminal swap rates and annuities
        firstCotSwap_ = nRates_;
    }



    const std::vector<Rate>& LMMCurveState::forwardRates() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        return forwardRates_;
    }

    const std::vector<Rate>&
    LMMCurveState::coterminalSwapAnnuities() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        if (firstCotSwap_>first_)
            computeCoterminalSwap(first_);
        return cotAnnuities_;
    }

    const std::vector<Rate>& LMMCurveState::coterminalSwapRates() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        if (firstCotSwap_>first_)
            computeCoterminalSwap(first_);
        return cotSwaps_;
    }

    Rate LMMCurveState::forwardRate(Size i) const {
        QL_REQUIRE(i>=first_, "index too low");
        QL_REQUIRE(i<nRates_, "index too high");
        return forwardRates_[i];
    }

    Real LMMCurveState::discountRatio(Size i, Size j) const {
        Size iMin = std::min(i, j);
        QL_REQUIRE(iMin>=first_, "index too low");
        QL_REQUIRE(std::max(i, j)<=nRates_, "index too high");
        return discRatios_[i]/discRatios_[j];
    }

    Rate LMMCurveState::coterminalSwapAnnuity(Size i) const {
        QL_REQUIRE(i>=first_, "index too low");
        QL_REQUIRE(i<nRates_, "index too high");
        if (firstCotSwap_>i)
            computeCoterminalSwap(i);
        return cotAnnuities_[i];
    }

    Rate LMMCurveState::coterminalSwapRate(Size i) const {
        QL_REQUIRE(i>=first_, "index too low");
        QL_REQUIRE(i<nRates_, "index too high");
        if (firstCotSwap_>i)
            computeCoterminalSwap(i);
        return cotSwaps_[i];
    }

    // Computation of coterminal swap rates and annuities
    // from discount ratios and fwd rates
    void LMMCurveState::computeCoterminalSwap(Size i) const {
        Real accumAnn = 0.0;
        if (firstCotSwap_!=nRates_)
            accumAnn = cotAnnuities_[firstCotSwap_];
        while (firstCotSwap_>i) {
            --firstCotSwap_;
            accumAnn += taus_[firstCotSwap_] * discRatios_[firstCotSwap_+1];
            cotAnnuities_[firstCotSwap_] = accumAnn;
            cotSwaps_[firstCotSwap_] =
                (discRatios_[firstCotSwap_]-discRatios_[nRates_])/accumAnn;
        }
    }

}
