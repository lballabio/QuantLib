/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Cristina Duminuco

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/curvestates/coterminalswapcurvestate.hpp>

namespace QuantLib {

    CoterminalSwapCurveState::CoterminalSwapCurveState(
                                        const std::vector<Time>& rateTimes)
    : CurveState(rateTimes),
      first_(numberOfRates_),
      discRatios_(numberOfRates_+1, 1.0),
      forwardRates_(numberOfRates_),
      cmSwapRates_(numberOfRates_),
      cmSwapAnnuities_(numberOfRates_, rateTaus_[numberOfRates_-1]),
      cotSwapRates_(numberOfRates_),
      cotAnnuities_(numberOfRates_, rateTaus_[numberOfRates_-1]) {}

      void CoterminalSwapCurveState::setOnCoterminalSwapRates(
                                        const std::vector<Rate>& rates,
                                        Size firstValidIndex) {
            QL_REQUIRE(rates.size()==numberOfRates_,
                       "rates mismatch: " <<
                       numberOfRates_ << " required, " <<
                       rates.size() << " provided");
            QL_REQUIRE(firstValidIndex<numberOfRates_,
                       "first valid index must be less than " <<
                       numberOfRates_ << ": " <<
                       firstValidIndex << " not allowed");
        // first copy input...
        first_ = firstValidIndex;
        std::copy(rates.begin()+first_, rates.end(),
                  cotSwapRates_.begin()+first_);
        // ...then calculate discount ratios and coterminal annuities:
        // reference discount bond =  P(n) (the last one)
        // discRatios_[numberOfRates_] = P(n)/P(n) = 1.0 by construction/definition
        cotAnnuities_[numberOfRates_-1] = rateTaus_[numberOfRates_-1];
        // j < n
        for (Size i=numberOfRates_-1; i>first_; --i) {
            discRatios_[i] = 1.0 + cotSwapRates_[i] * cotAnnuities_[i];
            cotAnnuities_[i-1] = cotAnnuities_[i] + rateTaus_[i-1] * discRatios_[i];
        }
        discRatios_[first_] = 1.0 + cotSwapRates_[first_] * cotAnnuities_[first_];

        // Insert here lazy evaluation of:
        // - forward rates
        // - constant maturity swap rates/annuities
    }

    Real CoterminalSwapCurveState::discountRatio(Size i, Size j) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(std::min(i, j)>=first_, "invalid index");
        QL_REQUIRE(std::max(i, j)<=numberOfRates_, "invalid index");
        return discRatios_[i]/discRatios_[j];
    }

    Rate CoterminalSwapCurveState::forwardRate(Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        forwardsFromDiscountRatios(first_, discRatios_, rateTaus_, forwardRates_);
        return forwardRates_[i];
    }

    Rate CoterminalSwapCurveState::coterminalSwapAnnuity(Size numeraire,
                                                         Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=numberOfRates_,
                  "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        return cotAnnuities_[i]/discRatios_[numeraire];
    }

    Rate CoterminalSwapCurveState::coterminalSwapRate(Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        return cotSwapRates_[i];
    }

    Rate CoterminalSwapCurveState::cmSwapAnnuity(Size numeraire,
                                                 Size i,
                                                 Size spanningForwards) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=numberOfRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");

        // consider lazy evaluation here
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, rateTaus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapAnnuities_[i]/discRatios_[numeraire];
    }

    Rate CoterminalSwapCurveState::cmSwapRate(Size i,
                                              Size spanningForwards) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");

        // consider lazy evaluation here
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, rateTaus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapRates_[i];
    }

    const std::vector<Rate>& CoterminalSwapCurveState::forwardRates() const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        forwardsFromDiscountRatios(first_, discRatios_, rateTaus_, forwardRates_);
        return forwardRates_;
    }

    const std::vector<Rate>& CoterminalSwapCurveState::coterminalSwapRates() const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        return cotSwapRates_;
    }

    const std::vector<Rate>& CoterminalSwapCurveState::cmSwapRates(Size spanningForwards) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, rateTaus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapRates_;
    }

    std::unique_ptr<CurveState>
    CoterminalSwapCurveState::clone() const {
        return std::unique_ptr<CurveState>(new CoterminalSwapCurveState(*this));
    }

}
