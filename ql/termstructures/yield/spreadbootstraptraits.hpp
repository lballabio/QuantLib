/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_spread_bootstrap_traits_hpp
#define quantlib_spread_bootstrap_traits_hpp

#include <ql/termstructures/yield/bootstraptraits.hpp>
#include <ql/termstructures/yield/spreaddiscountcurve.hpp>

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

}

#endif
