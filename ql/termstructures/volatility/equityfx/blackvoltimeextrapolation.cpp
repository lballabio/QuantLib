/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 AcadiaSoft Inc.
 Copyright (C) 2026 Paolo D'Elia

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

#include <ql/termstructures/volatility/equityfx/blackvoltimeextrapolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/errors.hpp>
#include <array>

namespace QuantLib {

    namespace {

        Real linearExtrapolation(Time t, const std::array<Time, 2>& times, const std::array<Real, 2>& variances) {
            QL_REQUIRE(t > times[1], "t must be greater than times[1]");
            QL_REQUIRE(times[1] > times[0], "times must be sorted");
            QL_REQUIRE(variances[1] >= variances[0], "variances must be non-decreasing");
            std::array<Real, 2> vols;
            vols[0] = close_enough(times[0], 0.0) ? 0.0 : std::sqrt(variances[0] / times[0]);
            vols[1] = close_enough(times[1], 0.0) ? 0.0 : std::sqrt(variances[1] / times[1]);
            LinearInterpolation interpolation(times.begin(), times.end(), vols.begin());
            return std::max(interpolation(t, true), 0.0);
        }

        Real timeExtrapolationBlackVarianceFlat(Time t, Real strike, const std::vector<double>& times,
                                                const std::function<Real(Time t, Real k)>& varianceSurface) {
            return std::max(varianceSurface(times.back(), strike), 0.0) / times.back() * t;
        }

        Real timeExtrapolationBlackVarianceFlat(Time t, const std::vector<double>& times,
                                                const std::function<Real(Time t)>& varianceCurve) {
            return std::max(varianceCurve(times.back()), 0.0) / times.back() * t;
        }

        Real timeExtrapolationBlackVarianceLinear(Time t, Real strike, const std::vector<double>& times,
                                                  const std::function<Real(Time t, Real k)>& varianceSurface) {
            Size ind1 = times.size() - 2;
            Size ind2 = times.size() - 1;
            std::array<Real, 2> xs{times[ind1], times[ind2]};
            std::array<Real, 2> variances;
            variances[0] = varianceSurface(xs[0], strike);
            variances[1] = varianceSurface(xs[1], strike);
            Real v = linearExtrapolation(t, xs, variances);
            return v * v * t;
        }

        Real timeExtrapolationBlackVarianceLinear(Time t, const std::vector<double>& times,
                                                  const std::function<Real(Time t)>& varianceCurve) {
            Size ind1 = times.size() - 2;
            Size ind2 = times.size() - 1;
            std::array<Real, 2> xs{times[ind1], times[ind2]};
            std::array<Real, 2> variances;
            variances[0] = varianceCurve(xs[0]);
            variances[1] = varianceCurve(xs[1]);
            Real v = linearExtrapolation(t, xs, variances);
            return v * v * t;
        }

    }

    Real BlackVolTimeExtrapolation::extrapolate(Type type, Time t, Real strike, const std::vector<Time>& times, const std::function<Real(Time t, Real k)>& varianceSurface) {
        switch (type) {
          case FlatVolatility:
            return timeExtrapolationBlackVarianceFlat(t, strike, times, varianceSurface);
          case UseInterpolatorVariance:
            return std::max(varianceSurface(t, strike), 0.0);
          case LinearVariance: 
            return timeExtrapolationBlackVarianceLinear(t, strike, times, varianceSurface);
          default:
            QL_FAIL("unknown extrapolation type");
        }
    }

    Real BlackVolTimeExtrapolation::extrapolate(Type type, Time t, const std::vector<Time>& times, const std::function<Real(Time t)>& varianceCurve) {
        switch (type) {
          case FlatVolatility:
            return timeExtrapolationBlackVarianceFlat(t, times, varianceCurve);
          case UseInterpolatorVariance:
            return std::max(varianceCurve(t), 0.0);
          case LinearVariance: {
              QL_REQUIRE(times.size() >= 2, "at least two times required for volatility extrapolation");
              return timeExtrapolationBlackVarianceLinear(t, times, varianceCurve);
          }
          default:
            QL_FAIL("unknown extrapolation type");
        }
    }

}

