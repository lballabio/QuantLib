/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti

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

#include <ql/MarketModels/curvestate.hpp>

namespace QuantLib {

    CurveState::CurveState(const std::vector<Time>& rateTimes)
    : rateTimes_(rateTimes), taus_(rateTimes.size()-1),
      forwardRates_(rateTimes.size()-1), discountRatios_(rateTimes.size()),
      coterminalSwaps_(rateTimes.size()-1), annuities_(rateTimes.size()-1),
      firstSwapComputed_(last_), first_(0), last_(rateTimes.size()-1) {

        for (Size i=first_; i<last_; i++) {
            taus_[i] = rateTimes_[i+1] - rateTimes_[i];
        }
    }

    void CurveState::setOnForwardRates(const std::vector<Rate>& rates)
    {
        // Note: already fixed forwards are left in the vector rates
        QL_REQUIRE(rates.size()==last_, "too many forward rates");
        std::copy(rates.begin(),rates.end(),forwardRates_.begin());
        // Computation of discount ratios
        discountRatios_[first_]=1.0;
        for (Size i=first_+1; i<=last_; i++) {
            discountRatios_[i] =
                discountRatios_[i-1]/(1.+taus_[i-1]*forwardRates_[i-1]);
        }
        // Reset coterminal swap rates to be calculated
        firstSwapComputed_ = last_;
    }

    void CurveState::setOnDiscountRatios(
                          const std::vector<DiscountFactor>& discountRatios) {
        // already fixed forwards are still in the vector rates
        QL_REQUIRE(discountRatios.size()==last_, "too many discount ratios");
        std::copy(discountRatios.begin(),discountRatios.end(),
                  discountRatios_.begin());
        // Computation of forward rates
        for (Size i=first_; i<last_; i++) {
            forwardRates_[i] =
                (discountRatios_[i]/discountRatios_[i+1]-1.)/taus_[i];
        }
        // Reset coterminal swap rates to be calculated
        firstSwapComputed_ = last_;
    }

    void CurveState::setOnCoterminalSwapRates(
                                         const std::vector<Rate>& swapRates) {
        QL_FAIL("not yet implemented");
        // todo fwd and discount ratios
        //QL_REQUIRE(swapRates.size()==last_, "too many swap rates");
        //std::copy(swapRates.begin(),swapRates.end(),
        //          coterminalSwaps_.begin());
        // etc.
    }

    void CurveState::computeSwapRate() const {
        // Compute backward the coterminal swap rates
        annuities_[last_-1] = taus_[last_-1]*discountRatios_[last_];
        coterminalSwaps_[last_-1] = forwardRates_[last_-1];
        firstSwapComputed_--;
        for (Size i=last_-1; i>first_; i--) {
            annuities_[i-1] = annuities_[i] + taus_[i-1]*discountRatios_[i];
            coterminalSwaps_[i-1] =
                (discountRatios_[i-1]-discountRatios_[last_])/annuities_[i-1];
            firstSwapComputed_--;
        }
    }

}

