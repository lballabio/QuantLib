/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

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

        template <class C>
        static Real discountFactorDerivative(Time t, const C* c) {
            // The curve data are multiplicative discount spreads, while
            // helper sensitivities are with respect to the final discount.
            return c->baseCurve()->discount(t);
        }

        template <class C>
        static std::vector<std::pair<Size, Real>> extrapolationNodeWeights(
            Time t, const C* c,
                                 const Interpolation& interpolation) {
            // The inherited weights scale with the full discount
            // B(t)*S(t), but the curve nodes only drive the spread
            // factor S(t). discountFactorDerivative() supplies B(t),
            // so divide the base discount back out.
            auto weights =
                Discount::extrapolationNodeWeights(t, c, interpolation);
            DiscountFactor baseDiscount = c->baseCurve()->discount(t, true);
            for (auto& [j, w] : weights)
                w /= baseDiscount;
            return weights;
        }
    };

}

#endif
