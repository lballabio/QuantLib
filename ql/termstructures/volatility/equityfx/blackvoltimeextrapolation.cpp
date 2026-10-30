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
#include <ql/errors.hpp>
#include <array>

namespace QuantLib {

    namespace {

        Real linearExtrapolation(Time t, Time t1, Time t2, Real v1, Real v2) {
            QL_REQUIRE(t > 0.0, "t must be greater than 0.0");
            QL_REQUIRE(t > t2, "t must be greater than times[1]");
            QL_REQUIRE(t2 > t1, "times must be sorted");
            QL_REQUIRE(v2 >= v1, "variances must be non-decreasing");
            return v1 + (t - t1) * (v2 - v1) / (t2 - t1);
        }

        Real timeExtrapolationBlackVarianceFlat(Time t, Real strike, const std::vector<Time>& times,
                                                const std::function<Real(Time t, Real k)>& varianceSurface) {
            return std::max(varianceSurface(times.back(), strike), 0.0) / times.back() * t;
        }

        Real timeExtrapolationBlackVarianceFlat(Time t, const std::vector<Time>& times,
                                                const std::function<Real(Time t)>& varianceCurve) {
            return std::max(varianceCurve(times.back()), 0.0) / times.back() * t;
        }

        Real timeExtrapolationBlackVarianceLinear(Time t, Real strike, const std::vector<Time>& times,
                                                  const std::function<Real(Time t, Real k)>& varianceSurface) {
            Size N = times.size();
            Time t1 = times[N-2], t2 = times[N-1];
            Real var1 = varianceSurface(t1, strike), var2 = varianceSurface(t2, strike);
            return linearExtrapolation(t, t1, t2, var1, var2);
        }

        Real timeExtrapolationBlackVarianceLinear(Time t, const std::vector<Time>& times,
                                                  const std::function<Real(Time t)>& varianceCurve) {
            Size N = times.size();
            Time t1 = times[N-2], t2 = times[N-1];
            Real var1 = varianceCurve(t1), var2 = varianceCurve(t2);
            return linearExtrapolation(t, t1, t2, var1, var2);
        }

    }

    Real BlackVolTimeExtrapolation::extrapolatedVariance(Type type, Time t, Real strike, const std::vector<Time>& times,
                                                         const std::function<Real(Time t, Real k)>& varianceSurface) {
        switch (type) {
          case FlatVolatility:
            return timeExtrapolationBlackVarianceFlat(t, strike, times, varianceSurface);
          case UseInterpolator:
            return std::max(varianceSurface(t, strike), 0.0);
          case LinearVariance: 
            return timeExtrapolationBlackVarianceLinear(t, strike, times, varianceSurface);
          default:
            QL_FAIL("unknown extrapolation type");
        }
    }

    Real BlackVolTimeExtrapolation::extrapolatedVariance(Type type, Time t, const std::vector<Time>& times,
                                                         const std::function<Real(Time t)>& varianceCurve) {
        switch (type) {
          case FlatVolatility:
            return timeExtrapolationBlackVarianceFlat(t, times, varianceCurve);
          case UseInterpolator:
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

