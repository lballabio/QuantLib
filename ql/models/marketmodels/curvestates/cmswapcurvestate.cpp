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
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/curvestates/cmswapcurvestate.hpp>

namespace QuantLib {

    CMSwapCurveState::CMSwapCurveState(const std::vector<Time>& rateTimes,
                                       Size spanningForwards)
    : CurveState(rateTimes),
      spanningFwds_(spanningForwards),
      first_(numberOfRates_),
      discRatios_(numberOfRates_+1, 1.0),
      forwardRates_(numberOfRates_),
      cmSwapRates_(numberOfRates_),
      cmSwapAnnuities_(numberOfRates_, rateTaus_[numberOfRates_-1]),
      irrCMSwapRates_(numberOfRates_),
      irrCMSwapAnnuities_(numberOfRates_, rateTaus_[numberOfRates_-1]),
      cotSwapRates_(numberOfRates_),
      cotAnnuities_(numberOfRates_, rateTaus_[numberOfRates_-1]) {}

    void CMSwapCurveState::setOnCMSwapRates(const std::vector<Rate>& rates,
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
                  cmSwapRates_.begin()+first_);
        // ...then calculate discount ratios and annuities

        // taken care at constructor time
        //discRatios_[numberOfRates_] = 1.0;
        //cmSwapAnnuities_[numberOfRates_-1] = rateTaus_[numberOfRates_-1];

        // assume i+1 and do i
        Integer oldAnnuityEndIndex = numberOfRates_;
        for (Size i=numberOfRates_-1; i>first_; --i)
        {
            // formula 6.1 Joshi Liesch
            Integer endIndex = std::min(i + spanningFwds_,numberOfRates_);
            Integer annuityEndIndex = std::min(i + spanningFwds_-1,numberOfRates_);

            discRatios_[i] = discRatios_[endIndex] +
            cmSwapRates_[i]*cmSwapAnnuities_[i];
            cmSwapAnnuities_[i-1]= cmSwapAnnuities_[i]
            +discRatios_[i] * rateTaus_[i-1];

            if (annuityEndIndex < oldAnnuityEndIndex)
              cmSwapAnnuities_[i-1]-=discRatios_[oldAnnuityEndIndex] * rateTaus_[oldAnnuityEndIndex-1 ];

            oldAnnuityEndIndex = annuityEndIndex;
        }
        Integer endIndex = std::min(first_ + spanningFwds_,numberOfRates_);

        discRatios_[first_]= discRatios_[endIndex] +
        cmSwapRates_[first_]*cmSwapAnnuities_[first_];

        // lazy evaluation of forward and coterminal swap rates&annuities
    }

    Real CMSwapCurveState::discountRatio(Size i, Size j) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(std::min(i, j)>=first_, "invalid index");
        QL_REQUIRE(std::max(i, j)<=numberOfRates_, "invalid index");
        return discRatios_[i]/discRatios_[j];
    }

    Rate CMSwapCurveState::forwardRate(Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        forwardsFromDiscountRatios(first_, discRatios_, rateTaus_, forwardRates_);
        return forwardRates_[i];
    }

    Rate CMSwapCurveState::coterminalSwapAnnuity(Size numeraire,
                                                 Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=numberOfRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotAnnuities_[i]/discRatios_[numeraire];
    }

    Rate CMSwapCurveState::coterminalSwapRate(Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_[i];
    }

    Rate CMSwapCurveState::cmSwapAnnuity(Size numeraire,
                                         Size i,
                                         Size spanningForwards) const{
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=numberOfRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
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
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
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
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        forwardsFromDiscountRatios(first_, discRatios_, rateTaus_, forwardRates_);
        return forwardRates_;
    }

    const std::vector<Rate>& CMSwapCurveState::coterminalSwapRates() const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_;
    }

    const std::vector<Rate>& CMSwapCurveState::cmSwapRates(Size spanningForwards) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        if (spanningForwards==spanningFwds_)
            return cmSwapRates_;
        else {
            constantMaturityFromDiscountRatios(spanningForwards, first_,
                                               discRatios_, rateTaus_,
                                               irrCMSwapRates_, irrCMSwapAnnuities_);
            return irrCMSwapRates_;
        }
    }

    std::unique_ptr<CurveState> CMSwapCurveState::clone() const {
        return std::unique_ptr<CurveState>(new CMSwapCurveState(*this));
    }

}
