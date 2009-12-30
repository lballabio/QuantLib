/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti
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

#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>

namespace QuantLib {

    LMMCurveState::LMMCurveState(const std::vector<Time>& rateTimes)
    : CurveState(rateTimes),
      first_(numberOfRates_),
      discRatios_(numberOfRates_+1, 1.0),
      forwardRates_(numberOfRates_),
      cmSwapRates_(numberOfRates_),
      cmSwapAnnuities_(numberOfRates_,rateTaus_[numberOfRates_-1]),
      cotSwapRates_(numberOfRates_),
      cotAnnuities_(numberOfRates_,
      rateTaus_[numberOfRates_-1]),
      firstCotAnnuityComped_(numberOfRates_)
    {}

    void LMMCurveState::setOnForwardRates(const std::vector<Rate>& rates,
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
                  forwardRates_.begin()+first_);
        // ...then calculate discount ratios

        // taken care at constructor time
        //discRatios_[numberOfRates_] = 1.0;
        for (Size i=first_; i<numberOfRates_; ++i)
            discRatios_[i+1]=discRatios_[i]/(1.0+forwardRates_[i]*rateTaus_[i]);

        // lazy evaluation of:
        // - coterminal swap rates/annuities
        // - constant maturity swap rates/annuities

        firstCotAnnuityComped_ = numberOfRates_;
    }

    void LMMCurveState::setOnDiscountRatios(const std::vector<DiscountFactor>& discRatios,
                                            Size firstValidIndex) {
        QL_REQUIRE(discRatios.size()==numberOfRates_+1,
                   "too many discount ratios: " <<
                   numberOfRates_+1 << " required, " <<
                   discRatios.size() << " provided");
        QL_REQUIRE(firstValidIndex<numberOfRates_,
                   "first valid index must be less than " <<
                   numberOfRates_+1 << ": " <<
                   firstValidIndex << " not allowed");

        // first copy input...
        first_ = firstValidIndex;
        std::copy(discRatios.begin()+first_, discRatios.end(),
                  discRatios_.begin()+first_);
        // ...then calculate forwards

        for (Size i=first_; i<numberOfRates_; ++i)
            forwardRates_[i] = (discRatios_[i]/discRatios_[i+1]-1.0) /
                                                                rateTaus_[i];

        // lazy evaluation of:
        // - coterminal swap rates/annuities
        // - constant maturity swap rates/annuities

        firstCotAnnuityComped_ = numberOfRates_;
    }

    Real LMMCurveState::discountRatio(Size i, Size j) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(std::min(i, j)>=first_, "invalid index");
        QL_REQUIRE(std::max(i, j)<=numberOfRates_, "invalid index");
        return discRatios_[i]/discRatios_[j];
    }

    Rate LMMCurveState::forwardRate(Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        return forwardRates_[i];
    }

    Rate LMMCurveState::coterminalSwapAnnuity(Size numeraire,
                                              Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(numeraire>=first_ && numeraire<=numberOfRates_,
                   "invalid numeraire");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        //        coterminalFromDiscountRatios(first_,
        //                                   discRatios_, rateTaus_,
        //                                 cotSwapRates_, cotAnnuities_);

        if (firstCotAnnuityComped_ <=i)
            return  cotAnnuities_[i]/discRatios_[numeraire];

        if (firstCotAnnuityComped_ == numberOfRates_)
        {
            cotAnnuities_[numberOfRates_-1] = rateTaus_[numberOfRates_-1]*discRatios_[numberOfRates_];
            --firstCotAnnuityComped_;
        }

        for (int j= static_cast<int>(firstCotAnnuityComped_)-1; j >=static_cast<int>(i); --j)
            cotAnnuities_[j] = cotAnnuities_[j+1]+rateTaus_[j]*discRatios_[j+1];

        firstCotAnnuityComped_ = i;

        return cotAnnuities_[i]/discRatios_[numeraire];
    }

    Rate LMMCurveState::coterminalSwapRate(Size i) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");
        //    coterminalFromDiscountRatios(first_,
        //                               discRatios_, rateTaus_,
        //                             cotSwapRates_, cotAnnuities_);
        //      return cotSwapRates_[i];

        Real res = (discRatios_[i]/ discRatios_[numberOfRates_] -1.0)/coterminalSwapAnnuity(numberOfRates_,i);
        return res;
    }

    Rate LMMCurveState::cmSwapAnnuity(Size numeraire,
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

    Rate LMMCurveState::cmSwapRate(Size i,
                                   Size spanningForwards) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        QL_REQUIRE(i>=first_ && i<=numberOfRates_, "invalid index");

        // consider lazy evaluation here
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, rateTaus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapRates_[i];
    }

    const std::vector<Rate>& LMMCurveState::forwardRates() const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        return forwardRates_;
    }

    const std::vector<Rate>& LMMCurveState::coterminalSwapRates() const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        coterminalFromDiscountRatios(first_,
                                     discRatios_, rateTaus_,
                                     cotSwapRates_, cotAnnuities_);
        return cotSwapRates_;
    }

    const std::vector<Rate>& LMMCurveState::cmSwapRates(Size spanningForwards) const {
        QL_REQUIRE(first_<numberOfRates_, "curve state not initialized yet");
        constantMaturityFromDiscountRatios(spanningForwards, first_,
                                           discRatios_, rateTaus_,
                                           cmSwapRates_, cmSwapAnnuities_);
        return cmSwapRates_;
    }

}
