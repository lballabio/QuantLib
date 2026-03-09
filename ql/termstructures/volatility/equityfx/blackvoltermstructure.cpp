/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <array>
#include <ql/math/comparison.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>

namespace QuantLib {

    namespace detail {
        Real linearExtrapolation(const double t, const std::array<double, 2>& times, const std::array<double, 2>& variances) {
            QL_REQUIRE(t > times[1], "t must be greater than times[1]");
            QL_REQUIRE(times[1] > times[0], "times must be sorted");
            QL_REQUIRE(variances[1] >= variances[0], "variances must be non-decreasing");
            std::array<double, 2> vols;
            vols[0] = close_enough(times[0], 0.0) ? 0.0 : std::sqrt(variances[0] / times[0]);
            vols[1] = close_enough(times[1], 0.0) ? 0.0 : std::sqrt(variances[1] / times[1]);
            LinearInterpolation interpolation(times.begin(), times.end(), vols.begin());
            return std::max(interpolation(t, true), 0.0);
        }

        template <typename F>
        Real timeExtrapolationBlackVarianceFlat(const Time t, const Real strike, const std::vector<double>& times,
                                                const F& varianceSurface) {
            return std::max(varianceSurface(times.back(), strike, true), 0.0) / times.back() * t;
        }

        Real timeExtrapolationBlackVarianceFlat(const Time t, const std::vector<double>& times,
                                                        const Interpolation& varianceCurve) {
            return std::max(varianceCurve(times.back(), true), 0.0) / times.back() * t;
        }

        template <typename F>
        Real timeExtrapolationBlackVarianceLinear(const Time t, const Real strike, const std::vector<double>& times,
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

        Real timeExtrapolationBlackVarianceLinear(const Time t, const std::vector<double>& times,
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
    }

    // BlackVolTimeExtrapolation implementation
    template<class F>
    Real BlackVolTimeExtrapolation::extrapolate(Type type, const Time t, const Real strike, const std::vector<Time>& times, const F& varianceSurface) {
        switch (type) {
            case FlatVolatility:
                return detail::timeExtrapolationBlackVarianceFlat(t, times, strike, varianceSurface);
            case UseInterpolatorVariance:
                return std::max(varianceSurface(t, strike, true), 0.0);
            case LinearVariance: 
                return detail::timeExtrapolationBlackVarianceLinear(t, strike, times, varianceSurface);
            default:
                QL_FAIL("unknown extrapolation type");
        }
    }

    Real BlackVolTimeExtrapolation::extrapolate(Type type, const Time t, const std::vector<Time>& times, const Interpolation& varianceCurve) {
        switch (type) {
            case FlatVolatility:
                return detail::timeExtrapolationBlackVarianceFlat(t, times, varianceCurve);
            case UseInterpolatorVariance:
                return std::max(varianceCurve(t, true), 0.0);
            case LinearVariance: {
                QL_REQUIRE(times.size() >= 2, "at least two times required for volatility extrapolation");
                return detail::timeExtrapolationBlackVarianceLinear(t, times, varianceCurve);
            }
            default:
                QL_FAIL("unknown extrapolation type");
        }
    }

    BlackVolTermStructure::BlackVolTermStructure(BusinessDayConvention bdc,
                                                 const DayCounter& dc)
    : VolatilityTermStructure(bdc, dc) {}

    BlackVolTermStructure::BlackVolTermStructure(const Date& refDate,
                                                 const Calendar& cal,
                                                 BusinessDayConvention bdc,
                                                 const DayCounter& dc)
    : VolatilityTermStructure(refDate, cal, bdc, dc) {}

    BlackVolTermStructure::BlackVolTermStructure(Natural settlDays,
                                                 const Calendar& cal,
                                                 BusinessDayConvention bdc,
                                                 const DayCounter& dc)
    : VolatilityTermStructure(settlDays, cal, bdc, dc) {}

    Volatility BlackVolTermStructure::blackForwardVol(const Date& date1,
                                                      const Date& date2,
                                                      Real strike,
                                                      bool extrapolate) const {
        // (redundant) date-based checks
        QL_REQUIRE(date1 <= date2,
                   date1 << " later than " << date2);
        checkRange(date2, extrapolate);

        // using the time implementation
        Time time1 = timeFromReference(date1);
        Time time2 = timeFromReference(date2);
        return blackForwardVol(time1, time2, strike, extrapolate);
    }

    Volatility BlackVolTermStructure::blackForwardVol(Time time1,
                                                      Time time2,
                                                      Real strike,
                                                      bool extrapolate) const {
        QL_REQUIRE(time1 <= time2,
                   time1 << " later than " << time2);
        checkRange(time2, extrapolate);
        checkStrike(strike, extrapolate);
        if (time2==time1) {
            if (time1==0.0) {
                Time epsilon = 1.0e-5;
                Real var = blackVarianceImpl(epsilon, strike);
                return std::sqrt(var/epsilon);
            } else {
                Time epsilon = std::min<Time>(1.0e-5, time1);
                Real var1 = blackVarianceImpl(time1-epsilon, strike);
                Real var2 = blackVarianceImpl(time1+epsilon, strike);
                QL_ENSURE(var2>=var1,
                          "variances must be non-decreasing");
                return std::sqrt((var2-var1)/(2*epsilon));
            }
        } else {
            Real var1 = blackVarianceImpl(time1, strike);
            Real var2 = blackVarianceImpl(time2, strike);
            QL_ENSURE(var2 >= var1,
                      "variances must be non-decreasing");
            return std::sqrt((var2-var1)/(time2-time1));
        }
    }

    Real BlackVolTermStructure::blackForwardVariance(const Date& date1,
                                                     const Date& date2,
                                                     Real strike,
                                                     bool extrapolate)
                                                                      const {
        // (redundant) date-based checks
        QL_REQUIRE(date1 <= date2,
                   date1 << " later than " << date2);
        checkRange(date2, extrapolate);

        // using the time implementation
        Time time1 = timeFromReference(date1);
        Time time2 = timeFromReference(date2);
        return blackForwardVariance(time1, time2, strike, extrapolate);
    }

    Real BlackVolTermStructure::blackForwardVariance(Time time1,
                                                     Time time2,
                                                     Real strike,
                                                     bool extrapolate) const {
        QL_REQUIRE(time1 <= time2,
                   time1 << " later than " << time2);
        checkRange(time2, extrapolate);
        checkStrike(strike, extrapolate);
        Real v1 = blackVarianceImpl(time1, strike);
        Real v2 = blackVarianceImpl(time2, strike);
        QL_ENSURE(v2 >= v1,
                  "variances must be non-decreasing");
        return v2-v1;
    }

    BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : BlackVolTermStructure(bdc, dc) {}

    BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                                                    const Date& refDate,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : BlackVolTermStructure(refDate, cal, bdc, dc) {}

    BlackVolatilityTermStructure::BlackVolatilityTermStructure(
                                                    Natural settlementDays,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : BlackVolTermStructure(settlementDays, cal, bdc, dc) {}

    BlackVarianceTermStructure::BlackVarianceTermStructure(
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : BlackVolTermStructure(bdc, dc) {}

    BlackVarianceTermStructure::BlackVarianceTermStructure(
                                                    const Date& refDate,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : BlackVolTermStructure(refDate, cal, bdc, dc) {}

    BlackVarianceTermStructure::BlackVarianceTermStructure(
                                                    Natural settlementDays,
                                                    const Calendar& cal,
                                                    BusinessDayConvention bdc,
                                                    const DayCounter& dc)
    : BlackVolTermStructure(settlementDays, cal, bdc, dc) {}

}
