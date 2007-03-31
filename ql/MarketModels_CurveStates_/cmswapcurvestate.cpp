/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2006, 2007 Ferdinando Ametrano
Copyright (C) 2007 François du Vignaud
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

#include <ql/MarketModels/CurveStates/cmswapcurvestate.hpp>

namespace QuantLib {

    CMSwapCurveState::CMSwapCurveState(const std::vector<Time>& rateTimes,
                                       Size spanningForwards)
    : CurveState(rateTimes),
      spanningFwds_(spanningForwards),
      first_(nRates_),
      discRatios_(nRates_+1, 1.0),
      forwardRates_(nRates_),
      cmSwapRates_(nRates_), cmSwapAnnuities_(nRates_, rateTaus_[nRates_-1]),
      irrCMSwapRates_(nRates_), irrCMSwapAnnuities_(nRates_, rateTaus_[nRates_-1]),
      cotSwapRates_(nRates_), cotAnnuities_(nRates_, rateTaus_[nRates_-1]) {}

    void CMSwapCurveState::setOnCMSwapRates(const std::vector<Rate>& rates,
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
                  cmSwapRates_.begin()+first_);
        // ...then calculate discount ratios and annuities

        // taken care at constructor time
        //discRatios_[nRates_] = 1.0;
        //cmSwapAnnuities_[nRates_-1] = rateTaus_[nRates_-1];

        // assume i+1 and do i
        Integer oldAnnuityEndIndex = nRates_;
        for (Size i=nRates_-1; i>first_; --i)
        {
            // formula 6.1 Joshi Liesch
            Integer endIndex = std::min(i + spanningFwds_,nRates_);
            Integer annuityEndIndex = std::min(i + spanningFwds_-1,nRates_);

            discRatios_[i] = discRatios_[endIndex] +
            cmSwapRates_[i]*cmSwapAnnuities_[i];
            cmSwapAnnuities_[i-1]= cmSwapAnnuities_[i]
            +discRatios_[i] * rateTaus_[i-1];

            if (annuityEndIndex < oldAnnuityEndIndex)
              cmSwapAnnuities_[i-1]-=discRatios_[oldAnnuityEndIndex] * rateTaus_[oldAnnuityEndIndex-1 ];

            oldAnnuityEndIndex = annuityEndIndex;
        }
        Integer endIndex = std::min(first_ + spanningFwds_,nRates_);

        discRatios_[first_]= discRatios_[endIndex] +
        cmSwapRates_[first_]*cmSwapAnnuities_[first_];

        // lazy evaluation of forward and coterminal swap rates&annuities
    }

    Real CMSwapCurveState::discountRatio(Size i, Size j) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(std::min(i, j)>=first_, "invalid index");
        QL_REQUIRE(std::max(i, j)<=nRates_, "invalid index");
        return discRatios_[i]/discRatios_[j];
    }

    Rate CMSwapCurveState::forwardRate(Size i) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        forwardsFromDiscountRatios(first_, discRatios_, rateTaus_, forwardRates_);
        return forwardRates_[i];
    }

    Rate CMSwapCurveState::coterminalSwapAnnuity(Size numeraire,
                                                 Size i) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=nRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotAnnuities_[i]/discRatios_[numeraire];
    }

    Rate CMSwapCurveState::coterminalSwapRate(Size i) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_[i];
    }

    Rate CMSwapCurveState::cmSwapAnnuity(Size numeraire,
                                         Size i,
                                         Size spanningForwards) const{
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=nRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        if (spanningForwards==spanningFwds_)
            return cmSwapAnnuities_[i]/discRatios_[numeraire];
        else {
            constantMaturityFromDiscountRatios(spanningForwards, first_,
                                               discRatios_, rateTaus_,
                                               irrCMSwapRates_,
                                               irrCMSwapAnnuities_);
            return irrCMSwapAnnuities_[i]/discRatios_[numeraire];
        }
    }

    Rate CMSwapCurveState::cmSwapRate(Size i,
                                      Size spanningForwards) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=nRates_, "invalid index");
        if (spanningForwards==spanningFwds_)
            return cmSwapRates_[i];
        else {
            constantMaturityFromDiscountRatios(spanningForwards, first_,
                                               discRatios_, rateTaus_,
                                               irrCMSwapRates_,
                                               irrCMSwapAnnuities_);
            return irrCMSwapRates_[i];
        }
    }

    const std::vector<Rate>& CMSwapCurveState::forwardRates() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        forwardsFromDiscountRatios(first_, discRatios_, rateTaus_, forwardRates_);
        return forwardRates_;
    }

    const std::vector<Rate>& CMSwapCurveState::coterminalSwapRates() const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_;
    }

    const std::vector<Rate>& CMSwapCurveState::cmSwapRates(Size spanningForwards) const {
        QL_REQUIRE(first_<nRates_, "curve state not initialized yet");
        if (spanningForwards==spanningFwds_)
            return cmSwapRates_;
        else {
            constantMaturityFromDiscountRatios(spanningForwards, first_,
                                               discRatios_, rateTaus_,
                                               irrCMSwapRates_, irrCMSwapAnnuities_);
            return irrCMSwapRates_;
        }
    }

}
