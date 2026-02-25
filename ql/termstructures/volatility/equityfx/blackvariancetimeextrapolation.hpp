/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 AcadiaSoft Inc.

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

/*! \file blackvariancetimeextrapolation.hpp
    \brief Utility function for time extrapolation in Black volatility in black variance term structures
*/

#include <array>
#include <functional>
#include <ql/math/comparison.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>

namespace QuantLib {

    namespace detail {
        Real linearExtrapolation(const double t, const std::array<double, 2>& times, const std::array<double, 2>& variances);

        inline Real linearExtrapolation(const double t, const std::array<double, 2>& times,
                                        const std::array<double, 2>& variances) {
            QL_REQUIRE(t > times[1], "t must be greater than times[1]");
            QL_REQUIRE(times[1] > times[0], "times must be sorted");
            QL_REQUIRE(variances[1] >= variances[0], "variances must be non-decreasing");
            std::array<double, 2> vols;
            vols[0] = close_enough(times[0], 0.0) ? 0.0 : std::sqrt(variances[0] / times[0]);
            vols[1] = close_enough(times[1], 0.0) ? 0.0 : std::sqrt(variances[1] / times[1]);
            LinearInterpolation interpolation(times.begin(), times.end(), vols.begin());
            return std::max(interpolation(t, true), 0.0);
        }
    } // namespace



    //! Extrapolate black variance using flat vol extrapolation in time direction
    Real timeExtrapolationBlackVarianceFlat(const Time t, const std::vector<double>& times,
        const Interpolation& varianceCurve);

    //! Extrapolate black variance using flat vol extrapolation in time direction
    template <typename F>
    Real timeExtrapolationBlackVarianceFlat(const Time t, const Real strike, const std::vector<double>& times,
                                            const F& varianceSurface) {
        return std::max(varianceSurface(times.back(), strike, true), 0.0) / times.back() * t;
    }


    //! Extrapolate black variance in vol space and time direction using interpolation
    //! Takes black variances convert them to volatilities and then linearly extrapolates
    //! the volatilities in time direction
    Real timeExtrapolationBlackVarianceInVolatility(const Time t, const std::vector<double>& times,
        const Interpolation& varianceCurve);

    //! Extrapolate black variance in vol space and time direction using interpolation
    //! Takes black variances convert them to volatilities and then linearly extrapolates
    //! the volatilities in time direction
    template <typename F>
    Real timeExtrapolationBlackVarianceInVolatility(const Time t, const Real strike, const std::vector<double>& times,
                                                    const F& varianceSurface) {
        Size ind1 = times.size() - 2;
        Size ind2 = times.size() - 1;
        std::array<Real, 2> xs{times[ind1], times[ind2]};
        std::array<Real, 2> variances;
        variances[0] = varianceSurface(xs[0], strike, true);
        variances[1] = varianceSurface(xs[1], strike, true);
        Real v = detail::linearExtrapolation(t, xs, variances);
        return v * v * t;
    }

    inline Real timeExtrapolationBlackVarianceFlat(const Time t, const std::vector<double>& times,
                                                    const Interpolation& varianceCurve) {
        return std::max(varianceCurve(times.back(), true), 0.0) / times.back() * t;
    }

    inline Real timeExtrapolationBlackVarianceInVolatility(const Time t, const std::vector<double>& times,
                                                            const Interpolation& varianceCurve) {
        Size ind1 = times.size() - 2;
        Size ind2 = times.size() - 1;
        std::array<Real, 2> xs{times[ind1], times[ind2]};
        std::array<Real, 2> variances;
        variances[0] = varianceCurve(xs[0], true);
        variances[1] = varianceCurve(xs[1], true);
        Real v = detail::linearExtrapolation(t, xs, variances);
        return v * v * t;
    }
} // namespace QuantLib