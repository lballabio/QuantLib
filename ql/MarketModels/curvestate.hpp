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
        CurveState(const Array& rateTimes);
     
        void setOnForwardRates(const Array& rates);
        void setOnDiscountRatios(const Array& discountRatios);
        void setOnCoterminalSwapRates(const Array& swapRates);

        // You should get an error if you look outside [first, last) range.
        /*
        void setOnForwardRates(const Array& rates, Size first, Size last);
        void setOnDiscountRatios(const Array& discountRatios,
                                 Size first, Size last);
        void setOnCoterminalSwaps(const Array& swapRates, Size first);
        */

        const Array& forwardRates() const;
        const Array& discountRatios() const;
        const Array& coterminalSwapRates() const;

        Rate forwardRate(Size i) const;
        Real discountRatio(Size i, Size j) const;
        Rate coterminalSwapRate(Size i) const;
    
    private:
        
        Array rateTimes_, taus_, forwardRates_, discountRatios_;
        mutable Array coterminalSwaps_, annuities_;
        mutable Size firstSwapComputed_;
        Size first_, last_;

        void computeSwapRate() const;

        // suggest lazy evaluation on the coterminal swaps 
        // e.g store index of how many swaps from the end have been computed
        // note: only makes sense if last_ is final time 


    };

}


#endif
