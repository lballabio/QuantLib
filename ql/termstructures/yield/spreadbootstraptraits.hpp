/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_spread_bootstrap_traits_hpp
#define quantlib_spread_bootstrap_traits_hpp

#include <ql/termstructures/yield/bootstraptraits.hpp>
#include <ql/termstructures/yield/spreaddiscountcurve.hpp>
#include <ql/termstructures/yield/spreadzerocurve.hpp>

namespace QuantLib::detail {

    template <class Traits>
    struct SpreadTraits;

    //! Spread Discount-curve traits
    template <>
    struct SpreadTraits<Discount> : Discount {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedSpreadDiscountCurve<Interpolator> type;
        };
    };

    //! Spread Zero-curve traits
    template <>
    struct SpreadTraits<ZeroYield> : ZeroYield {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedSpreadZeroCurve<Interpolator> type;
        };

        // value at reference date: zero spread
        static Real initialValue(const YieldTermStructure*) {
            return 0.0;
        }

        // guesses
        template <class C>
        static Real guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];
            return 0.0;
        }
    };

}

#endif
