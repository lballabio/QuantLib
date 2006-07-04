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

    CurveState::CurveState(const Array& rateTimes) : rateTimes_(rateTimes), 
                                                     first_(0), last_(rateTimes.size()-1),
                                                     forwardRates_(rateTimes.size()-1),
                                                     discountRatios_(rateTimes.size()),
                                                     coterminalSwaps_(rateTimes.size()-1),
                                                     annuities_(rateTimes.size()-1),
                                                     taus_(rateTimes.size()-1),
                                                     firstSwapComputed_(last_) {

/* There will n+1 rate times expressing payment and reset times of forward rates.

               |-----|-----|-----|-----|-----|      (size = 6)
               t0    t1    t2    t3    t4    t5     rateTimes
               f0    f1    f2    f3    f4           forwardRates
               d0    d1    d2    d3    d4    d5     discountBonds 
               d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios
               sr0   sr1   sr2   sr3   sr4          coterminalSwaps
*/
        for (Size i=first_; i<last_; i++) {
            taus_[i] = rateTimes_[i+1] - rateTimes_[i];
        }
    }
     
    void CurveState::setOnForwardRates(const Array& rates) {
        // Note: already fixed forwards are left in the vector rates
        QL_REQUIRE(rates.size()==last_, "too many forward rates");       
        std::copy(rates.begin(),rates.end(),forwardRates_.begin());
        // Computation of discount ratios
        discountRatios_[first_]=1.0; 
        for (Size i=first_+1; i<=last_; i++) {
            discountRatios_[i] = discountRatios_[i-1]/(1.+taus_[i-1]*forwardRates_[i-1]);
        } 
        // Reset coterminal swap rates to be calculated
        firstSwapComputed_ = last_;
    }

    void CurveState::setOnDiscountRatios(const Array& discountRatios) {
        // already fixed forwards are still in the vector rates (they remain constant)
        QL_REQUIRE(discountRatios.size()==last_, "too many discount ratios");       
        std::copy(discountRatios.begin(),discountRatios.end(),discountRatios_.begin());
        // Computation of forward rates
        for (Size i=first_; i<last_; i++) {
            forwardRates_[i] = (discountRatios_[i]/discountRatios_[i+1]-1.)/taus_[i];
        } 
        // Reset coterminal swap rates to be calculated
        firstSwapComputed_ = last_;
    }
    
    void CurveState::setOnCoterminalSwaps(const Array& swapRates) {
        QL_FAIL("not yet implemented");
        // todo fwd and discount ratios
        //QL_REQUIRE(swapRates.size()==last_, "too many swap rates");       
        //std::copy(swapRates.begin(),swapRates.end(),coterminalSwaps_.begin());
        // etc.
    }

    void CurveState::computeSwapRate() const
    {
        // Compute backward the coterminal swap rates
        annuities_[last_-1] = taus_[last_-1]*discountRatios_[last_];
        coterminalSwaps_[last_-1] = forwardRates_[last_-1];
        firstSwapComputed_--;
        for (Size i=last_-1; i>first_; i--) {
            annuities_[i-1] = annuities_[i] + taus_[i-1]*discountRatios_[i];
            coterminalSwaps_[i-1] = (discountRatios_[i-1]-discountRatios_[last_])/annuities_[i-1];
            firstSwapComputed_--;
        }
    }

    const Array& CurveState::forwardRates() const {
        return forwardRates_;
    }
        
    const Array& CurveState::discountRatios() const {
        return discountRatios_;
    }
    
    const Array& CurveState::coterminalSwapRates() const {
        if (firstSwapComputed_>first_) {
            computeSwapRate();
        }
        return coterminalSwaps_;
    }

    Rate CurveState::forwardRate(Size i) const {
        return forwardRates_[i];
    }
    
    Rate CurveState::coterminalSwapRate(Size i) const {
        return coterminalSwapRates()[i];
    }
        
    Real CurveState::discountRatio(Size i, Size j) const {
        return discountRatios_[i]/discountRatios_[j];
    }


}
