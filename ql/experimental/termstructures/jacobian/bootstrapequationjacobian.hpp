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

/*! \file bootstrapequationjacobian.hpp
    \brief Jacobian of a single bootstrapped curve
*/

#ifndef quantlib_experimental_bootstrap_equation_jacobian_hpp
#define quantlib_experimental_bootstrap_equation_jacobian_hpp

#include <ql/math/interpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class T, class = void>
        constexpr bool supportsAnalyticJacobian = false;

        template <class T>
        constexpr bool supportsAnalyticJacobian<
            T, std::void_t<decltype(T::supportsAnalyticJacobian)>> =
                T::supportsAnalyticJacobian;

        // access to curve internals for cross-curve Jacobians
        template <class Curve>
        struct BootstrapJacobianAccess;

        /*! Converts dated curve-value sensitivities into one Jacobian row.
            Returns false when analytical weights are unavailable.
        */
        template <class Traits, class Curve>
        bool analyticBootstrapEquationRow(const Curve* curve,
                             const std::vector<std::pair<Time, Real>>& sensitivities,
                             const std::vector<Time>& times,
                             const Interpolation& interpolation,
                             std::vector<Real>& row) {
            if constexpr (!supportsAnalyticJacobian<Traits>) {
                return false;
            } else {
                constexpr bool firstTied = Traits::firstDataPointTracksSecond;

                Time tMax = times.back();
                row.resize(times.size() - 1);
                std::fill(row.begin(), row.end(), 0.0);
                for (const auto& [t, dQdP] : sensitivities) {
                    std::vector<std::pair<Size, Real>> weights;
                    if (t <= tMax) {
                        weights = interpolation.nodeWeights(t, true);
                    } else {
                        // The curve tail can differ from the interpolation tail.
                        weights = Traits::extrapolationNodeWeights(
                            t, curve, interpolation);
                    }
                    if (weights.empty())
                        return false;
                    Real discountDerivative =
                        Traits::discountFactorDerivative(t, curve);
                    for (const auto& [j, w] : weights) {
                        if (j > 0)
                            row[j-1] += dQdP * discountDerivative * w;
                        else if (firstTied)
                            // data[0] tracks data[1]
                            row[0] += dQdP * discountDerivative * w;
                        // otherwise data[0] is fixed
                    }
                }
                return true;
            }
        }

        //! bootstrap-equation Jacobian and its calculation metadata
        struct BootstrapJacobian {
            Matrix matrix;
            std::vector<bool> analyticEquations;
        };

        //! Jacobian of the bootstrap equations with respect to curve nodes
        template <class Traits, class Curve>
        BootstrapJacobian bootstrapEquationJacobian(
                 const Curve* curve,
                 const std::vector<ext::shared_ptr<typename Traits::helper>>& instruments,
                 const std::vector<Time>& times,
                 std::vector<Real>& data,
                 Interpolation& interpolation,
                 bool curveHasJumps,
                 bool numericalFallback = true) {

            // alive helpers in curve order
            Date firstDate = Traits::initialDate(curve);
            std::vector<ext::shared_ptr<typename Traits::helper>> alive;
            for (const auto& helper : instruments)
                if (helper->pillarDate() > firstDate) {
                    QL_REQUIRE(helper->termStructure() == curve,
                               "the helper with pillar date " <<
                               helper->pillarDate() << " is seated to a "
                               "different curve; a rate helper cannot be "
                               "shared between bootstrapped curves");
                    alive.push_back(helper);
                }

            Size rows = alive.size();
            Size cols = times.size() - 1;
            Matrix J(rows, cols, 0.0);
            std::vector<bool> analytic(rows, false);

            if constexpr (supportsAnalyticJacobian<Traits>) {
                // analytical weights do not include jumps
                if (!curveHasJumps) {
                    std::vector<Real> row(cols);
                    for (Size i = 0; i < rows; ++i) {
                        auto sensitivities = alive[i]->impliedQuoteSensitivities();
                        if (sensitivities.empty())
                            continue;
                        if (analyticBootstrapEquationRow<Traits>(
                                curve, sensitivities, times, interpolation, row)) {
                            std::copy(row.begin(), row.end(), J.row_begin(i));
                            analytic[i] = true;
                        }
                    }
                }
            }

            // differentiate remaining rows one column at a time
            std::vector<Size> numericalRows;
            if (numericalFallback)
                for (Size i = 0; i < rows; ++i)
                    if (!analytic[i])
                        numericalRows.push_back(i);

            if (!numericalRows.empty()) {
                std::vector<Real> savedData = data;

                // restore in place to preserve interpolation iterators
                // and restore after an exception
                struct RestoreData {  // NOLINT(cppcoreguidelines-special-member-functions)
                    std::vector<Real>& data;
                    const std::vector<Real>& saved;
                    Interpolation& interpolation;
                    ~RestoreData() {
                        std::copy(saved.begin(), saved.end(), data.begin());
                        interpolation.update();
                    }
                } restore{data, savedData, interpolation};

                std::vector<Real> up(numericalRows.size());
                for (Size j = 1; j <= cols; ++j) {
                    Real v = savedData[j];
                    Real h = 1.0e-6 * std::max(std::abs(v), 0.01);

                    Traits::updateGuess(data, v + h, j);
                    interpolation.update();
                    for (Size k = 0; k < numericalRows.size(); ++k)
                        up[k] = alive[numericalRows[k]]->impliedQuote();

                    Traits::updateGuess(data, v - h, j);
                    interpolation.update();
                    for (Size k = 0; k < numericalRows.size(); ++k)
                        J[numericalRows[k]][j-1] =
                            (up[k] - alive[numericalRows[k]]->impliedQuote())/(2.0*h);

                    std::copy(savedData.begin(), savedData.end(), data.begin());
                    interpolation.update();
                }
            }

            return {std::move(J), std::move(analytic)};
        }

        //! invert a square bootstrap-equation Jacobian
        inline Matrix inverseBootstrapEquationJacobian(const Matrix& J) {
            QL_REQUIRE(J.rows() == J.columns(),
                       "cannot invert the Jacobian: the curve has " <<
                       J.columns() << " free nodes but only " << J.rows() <<
                       " alive helpers");
            return inverse(J);
        }

    }

}

#endif
