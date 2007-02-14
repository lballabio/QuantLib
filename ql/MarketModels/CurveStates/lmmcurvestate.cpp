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
    : CurveState(rateTimes),
      first_(nRates_),
      discRatios_(nRates_+1, 1.0),
      forwardRates_(nRates_),
      cmSwapRates_(nRates_), cmSwapAnnuities_(nRates_,taus_[nRates_-1]),
      cotSwapRates_(nRates_), cotAnnuities_(nRates_, taus_[nRates_-1]) {}

    void LMMCurveState::setOnForwardRates(const std::vector<Rate>& rates,
                                          Size firstValidIndex) {
            QL_REQUIRE(rates.size()==nRates_,
                       "rates mismatch: " <<
                       nRates_ << " required, " <<
                       rates.size() << " provided");
            QL_REQUIRE(firstValidIndex<nRates_,
                       "first valid index must be less than " <<
                       nRates_ << ": " <<
                       firstValidIndex << " not allowed");

        // first copy input...
        first_ = firstValidIndex;
        std::copy(rates.begin()+first_, rates.end(),
                  forwardRates_.begin()+first_);
        // ...then calculate discount ratios

        // taken care at constructor time
        //discRatios_[nRates_] = 1.0;
        for (Size i=first_; i<nRates_; ++i)
            discRatios_[i+1]=discRatios_[i]/(1.0+forwardRates_[i]*taus_[i]);

        // lazy evaluation of:
        // - coterminal swap rates/annuities
        // - constant maturity swap rates/annuities
    }

    void LMMCurveState::setOnDiscountRatios(const std::vector<DiscountFactor>& discRatios,
                                            Size firstValidIndex) {
        QL_REQUIRE(discRatios.size()==nRates_+1,
                   "too many discount ratios: " <<
                   nRates_+1 << " required, " <<
                   discRatios.size() << " provided");
        QL_REQUIRE(firstValidIndex<nRates_,
                   "first valid index must be less than " <<
                   nRates_+1 << ": " <<
                   firstValidIndex << " not allowed");

        // first copy input...
        first_ = firstValidIndex;
        std::copy(discRatios.begin()+first_, discRatios.end(),
                  discRatios_.begin()+first_);
        // ...then calculate forwards

        for (Size i=first_; i<nRates_; ++i)
            forwardRates_[i] = (discRatios_[i]/discRatios_[i+1]-1.0) /
                                                                taus_[i];

        // lazy evaluation of:
        // - coterminal swap rates/annuities
        // - constant maturity swap rates/annuities
    }

    Real LMMCurveState::discountRatio(Size i, Size j) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(std::min(i, j)>=first_, "invalid index");
        QL_REQUIRE(std::max(i, j)<=nRates_, "invalid index");
        return discRatios_[i]/discRatios_[j];
    }

    Rate LMMCurveState::forwardRate(Size i) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        return forwardRates_[i];
    }

    Rate LMMCurveState::coterminalSwapAnnuity(Size numeraire,
                                              Size i) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=nRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, taus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotAnnuities_[i]/discRatios_[numeraire];
    }

    Rate LMMCurveState::coterminalSwapRate(Size i) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, taus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_[i];
    }

    Rate LMMCurveState::cmSwapAnnuity(Size numeraire,
                                      Size i,
                                      Size spanningForwards) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=nRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");

        // consider lazy evaluation here
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, taus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapAnnuities_[i]/discRatios_[numeraire];
    }

    Rate LMMCurveState::cmSwapRate(Size i,
                                   Size spanningForwards) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");

        // consider lazy evaluation here
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, taus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapRates_[i];
    }

    const std::vector<Rate>& LMMCurveState::forwardRates() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        return forwardRates_;
    }

    const std::vector<Rate>& LMMCurveState::coterminalSwapRates() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, taus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_;
    }

    const std::vector<Rate>& LMMCurveState::cmSwapRates(Size spanningForwards) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, taus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapRates_;
    }

}
