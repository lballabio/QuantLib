
/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bootstraptraits.hpp
    \brief bootstrap traits
*/

#ifndef ql_bootstrap_traits_hpp
#define ql_bootstrap_traits_hpp

#include <ql/TermStructures/discountcurve.hpp>
#include <ql/TermStructures/zerocurve.hpp>

namespace QuantLib {

    //! Discount-curve traits
    struct Discount {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedDiscountCurve<Interpolator> type;
        };
        // value at reference
        static DiscountFactor initialValue() { return 1.0; }
        // initial guess
        static DiscountFactor initialGuess() { return 0.9; }
        // further guesses
        static DiscountFactor guess(const YieldTermStructure* curve,
                                    const Date& d) {
            return curve->discount(d,true);
        }
        // possible constraint based on previous values
        static DiscountFactor maxValueAfter(Size i,
                                            const std::vector<Real>& data) {
            #if defined(QL_NEGATIVE_RATES)
            // discount are not required to be decreasing--all bets are off.
            // We choose as max a value very unlikely to be exceeded.
            return 3.0;
            #else
            // discounts cannot increase
            return data[i-1];
            #endif
        }
        // update with new guess
        static void updateGuess(std::vector<DiscountFactor>& data,
                                DiscountFactor discount,
                                Size i) {
            data[i] = discount;
        }
    };


    //! Zero-curve traits
    struct ZeroYield {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedZeroCurve<Interpolator> type;
        };
        // (dummy) value at reference
        static Rate initialValue() { return 0.02; }
        // initial guess
        static Rate initialGuess() { return 0.02; }
        // further guesses
        static Rate guess(const YieldTermStructure* curve,
                          const Date& d) {
            return curve->zeroRate(d,curve->dayCounter(),
                                   Continuous,Annual,true);
        }
        // possible constraint based on previous values
        static DiscountFactor maxValueAfter(Size i,
                                            const std::vector<Real>& data) {
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return 3.0;
        }
        // update with new guess
        static void updateGuess(std::vector<Rate>& data,
                                Rate rate,
                                Size i) {
            data[i] = rate;
            if (i == 1)
                data[0] = rate; // first point is updated as well
        }
    };

}


#endif
