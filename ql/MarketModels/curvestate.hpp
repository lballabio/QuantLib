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


#ifndef quantlib_curvestate_hpp
#define quantlib_curvestate_hpp

#include <ql/Math/array.hpp>
#include <vector>

namespace QuantLib {

    
    /*! This class stores the state of the yield curve associated to the
        fixed calendar times within the simulation.

        This is the workhorse discounting object associated to the rate times
        of the simulation. It's important to pass the rates via an object like
        this to the product rather than directly to make it easier to switch
        to other engines such as a coterminal-swap-rate engine.
    
        Many products will not need expired rates and others will only require
        the first rate. 

    */
    class CurveState
    {
    public:
        CurveState(const std::vector<Time>& rateTimes);
     
        const std::vector<Time>& rateTimes() const;

        void setOnForwardRates(const std::vector<Rate>& rates);
        void setOnDiscountRatios(const std::vector<DiscountFactor>& discountRatios);
        void setOnCoterminalSwapRates(const std::vector<Rate>& swapRates);

        // You should get an error if you look outside [first, last) range.
        /*
        void setOnForwardRates(const std::vector<Rate>& rates, Size first, Size last);
        void setOnDiscountRatios(const std::vector<DiscountFactor>& discountRatios,
                                 Size first, Size last);
        void setOnCoterminalSwaps(const std::vector<Rate>&swapRates, Size first);
        */

        const std::vector<Rate>& forwardRates() const;
        const std::vector<DiscountFactor>& discountRatios() const;
        const std::vector<Rate>& coterminalSwapRates() const;

        Rate forwardRate(Size i) const;
        Real discountRatio(Size i, Size j) const;
        Rate coterminalSwapRate(Size i) const;
    
    private:
        
        std::vector<Time> rateTimes_, taus_;
        std::vector<Rate> forwardRates_;
        std::vector<DiscountFactor> discountRatios_;
        mutable std::vector<Rate> coterminalSwaps_;
        mutable std::vector<Real> annuities_;
        mutable Size firstSwapComputed_;
        Size first_, last_;

        // suggest lazy evaluation on the coterminal swaps 
        // e.g store index of how many swaps from the end have been computed
        // note: only makes sense if last_ is final time 
        void computeSwapRate() const;

    };

    // inline

    inline const std::vector<Time>& CurveState::rateTimes() const {
        return rateTimes_;
    }

    inline const std::vector<Rate>& CurveState::forwardRates() const {
        return forwardRates_;
    }

    inline const std::vector<DiscountFactor>& CurveState::discountRatios() const {
        return discountRatios_;
    }

    inline const std::vector<Rate>& CurveState::coterminalSwapRates() const {
        if (firstSwapComputed_>first_)
            computeSwapRate();

        return coterminalSwaps_;
    }

    inline Rate CurveState::forwardRate(Size i) const {
        return forwardRates_[i];
    }

    inline Rate CurveState::coterminalSwapRate(Size i) const {
        return coterminalSwapRates()[i];
    }

    inline Real CurveState::discountRatio(Size i, Size j) const {
        return discountRatios_[i]/discountRatios_[j];
    }

}

#endif
