/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

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

#include <ql/models/shortrate/twofactormodels/gaussian2dmodel.hpp>
#include <cmath>

namespace QuantLib {

    Real Gaussian2dModel::forwardRate(const Date& fixing,
                                      const Date& referenceDate,
                                      const Real yRate,
                                      const ext::shared_ptr<IborIndex>& iborIdx) const {

        QL_REQUIRE(iborIdx != nullptr, "no ibor index given");

        calculate();

        if (fixing <= (evaluationDate_ + (enforcesTodaysHistoricFixings_ ? 0 : -1)))
            return iborIdx->fixing(fixing);

        Handle<YieldTermStructure> yts =
            iborIdx->forwardingTermStructure(); // might be empty, then use model curve

        Date valueDate = iborIdx->valueDate(fixing);
        Date endDate = iborIdx->fixingCalendar().advance(
            valueDate, iborIdx->tenor(), iborIdx->businessDayConvention(), iborIdx->endOfMonth());
        Real dcf = iborIdx->dayCounter().yearFraction(valueDate, endDate);

        // Forward rate uses ONLY the rate factor (forecast, not discount)
        return (forecastZerobond(valueDate, referenceDate, yRate, yts) -
                forecastZerobond(endDate, referenceDate, yRate, yts)) /
               (dcf * forecastZerobond(endDate, referenceDate, yRate, yts));
    }

    Array Gaussian2dModel::yGrid(const ext::shared_ptr<StochasticProcess1D>& process,
                                  const Real stdDevs, const int gridPoints,
                                  const Real T, const Real t, const Real y) const {

        QL_REQUIRE(process != nullptr, "state process not set");

        Array result(2 * gridPoints + 1, 0.0);

        Real stdDev_0_T = process->stdDeviation(0.0, 0.0, T);
        Real e_0_T = process->expectation(0.0, 0.0, T);

        Real stdDev_t_T, e_t_T;

        if (t < QL_EPSILON) {
            stdDev_t_T = stdDev_0_T;
            e_t_T = e_0_T;
        } else {
            Real stdDev_0_t = process->stdDeviation(0.0, 0.0, t);
            stdDev_t_T = process->stdDeviation(t, 0.0, T - t);
            Real e_0_t = process->expectation(0.0, 0.0, t);
            Real x_t = y * stdDev_0_t + e_0_t;
            e_t_T = process->expectation(t, x_t, T - t);
        }

        Real h = stdDevs / static_cast<Real>(gridPoints);

        for (int j = -gridPoints; j <= gridPoints; j++) {
            result[j + gridPoints] =
                (e_t_T + stdDev_t_T * static_cast<Real>(j) * h - e_0_T) / stdDev_0_T;
        }

        return result;
    }

    Array Gaussian2dModel::yGrid(const Real stdDevs, const int gridPoints) const {
        // Standardized grid: just evenly spaced points from -stdDevs to +stdDevs
        Array result(2 * gridPoints + 1, 0.0);
        Real h = stdDevs / static_cast<Real>(gridPoints);
        for (int j = -gridPoints; j <= gridPoints; j++) {
            result[j + gridPoints] = static_cast<Real>(j) * h;
        }
        return result;
    }

    // Gaussian polynomial integration — same as Gaussian1dModel static methods

    Real Gaussian2dModel::gaussianPolynomialIntegral(
        const Real a, const Real b, const Real c, const Real d, const Real e,
        const Real y0, const Real y1) {

        const Real aa = 4.0 * a, ba = 2.0 * M_SQRT2 * b, ca = 2.0 * c,
                   da = M_SQRT2 * d;
        const Real x0 = y0 * M_SQRT1_2, x1 = y1 * M_SQRT1_2;
        return (0.125 * (3.0 * aa + 2.0 * ca + 4.0 * e) * std::erf(x1) -
                1.0 / (4.0 * M_SQRTPI) * std::exp(-x1 * x1) *
                    (2.0 * aa * x1 * x1 * x1 + 3.0 * aa * x1 +
                     2.0 * ba * (x1 * x1 + 1.0) + 2.0 * ca * x1 + 2.0 * da)) -
               (0.125 * (3.0 * aa + 2.0 * ca + 4.0 * e) * std::erf(x0) -
                1.0 / (4.0 * M_SQRTPI) * std::exp(-x0 * x0) *
                    (2.0 * aa * x0 * x0 * x0 + 3.0 * aa * x0 +
                     2.0 * ba * (x0 * x0 + 1.0) + 2.0 * ca * x0 + 2.0 * da));
    }

    Real Gaussian2dModel::gaussianShiftedPolynomialIntegral(
        const Real a, const Real b, const Real c, const Real d, const Real e,
        const Real h, const Real x0, const Real x1) {

        return gaussianPolynomialIntegral(
            a, -4.0 * a * h + b, 6.0 * a * h * h - 3.0 * b * h + c,
            -4.0 * a * h * h * h + 3.0 * b * h * h - 2.0 * c * h + d,
            a * h * h * h * h - b * h * h * h + c * h * h - d * h + e, x0, x1);
    }
}
