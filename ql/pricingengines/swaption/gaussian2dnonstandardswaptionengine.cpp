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

#include <ql/pricingengines/swaption/gaussian2dnonstandardswaptionengine.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/rebatedexercise.hpp>
#include <ql/utilities/null.hpp>

using std::exp;

namespace QuantLib {

    // ================================================================
    // SplineIntegrator: precomputes LU factorization of the natural
    // cubic spline tridiagonal system on a fixed uniform grid, then
    // reuses it for fast repeated Gaussian polynomial integration.
    //
    // For a uniform grid with spacing h, the natural cubic spline
    // second derivatives m_i satisfy:
    //   h·m_{i-1} + 4h·m_i + h·m_{i+1} = (6/h)(y_{i+1} - 2y_i + y_{i-1})
    // with m_0 = m_n = 0.
    //
    // The spline on [x_i, x_{i+1}] is:
    //   S(x) = a_i + b_i·(x-x_i) + c_i·(x-x_i)² + d_i·(x-x_i)³
    // where:
    //   a_i = y_i
    //   b_i = (y_{i+1}-y_i)/h - h·(m_{i+1}+2·m_i)/6
    //   c_i = m_i/2
    //   d_i = (m_{i+1}-m_i)/(6h)
    //
    // The Gaussian integral uses gaussianShiftedPolynomialIntegral
    // which expects coefficients of p(x) = A(x-h)^4+B(x-h)^3+C(x-h)^2+D(x-h)+E
    // but we pass A=0 (cubic, not quartic) and map c→C, d_i→... etc.
    // Actually, the 1D engine passes (0, c_coeff, b_coeff, a_coeff, value, shift).
    // For CubicInterpolation: aCoefficients = slope, bCoefficients = c_i (quadratic),
    // cCoefficients = d_i (cubic).
    // ================================================================

    class SplineIntegrator {
      public:
        explicit SplineIntegrator(const Array& z)
            : n_(z.size()), z_(z),
              // Pre-allocate working arrays
              m_(n_, 0.0), rhs_(n_, 0.0),
              lower_(n_, 0.0), diag_(n_, 0.0), upper_(n_, 0.0),
              aCoeff_(n_ - 1), bCoeff_(n_ - 1), cCoeff_(n_ - 1) {

            QL_REQUIRE(n_ >= 3, "need at least 3 grid points");
            h_ = z_[1] - z_[0]; // uniform grid
            h_inv_ = 1.0 / h_;
            six_h_inv_ = 6.0 * h_inv_;

            // LU factorize the tridiagonal system (Thomas algorithm)
            // Natural spline: m_0 = m_{n-1} = 0
            // Interior equations: h*m_{i-1} + 4h*m_i + h*m_{i+1} = rhs_i
            // We solve for m_1 ... m_{n-2} (n-2 unknowns)

            Size inner = n_ - 2;
            if (inner == 0) return;

            // Set up tridiagonal: diag=4h, off-diag=h
            for (Size i = 0; i < inner; i++) {
                diag_[i] = 4.0 * h_;
                lower_[i] = h_;
                upper_[i] = h_;
            }

            // Forward elimination (Thomas algorithm)
            for (Size i = 1; i < inner; i++) {
                Real w = lower_[i] / diag_[i - 1];
                diag_[i] -= w * upper_[i - 1];
                lower_[i] = w; // store multiplier for back-sub
            }
        }

        //! Compute ∫ S(z)·φ(z) dz where S is the natural cubic spline
        //! through (z, values) and φ is the standard normal density.
        Real integrate(const Array& values, bool isCall,
                       bool extrapolate, bool flatExtrapolation) const {

            Size inner = n_ - 2;

            // Compute RHS: (6/h)(y_{i+1} - 2y_i + y_{i-1}) for i=1..n-2
            for (Size i = 0; i < inner; i++)
                rhs_[i] = six_h_inv_ * (values[i + 2] - 2.0 * values[i + 1] + values[i]);

            // Forward substitution
            for (Size i = 1; i < inner; i++)
                rhs_[i] -= lower_[i] * rhs_[i - 1];

            // Back substitution → m_[1..n-2]
            m_[0] = 0.0;
            m_[n_ - 1] = 0.0;
            if (inner > 0) {
                m_[inner] = rhs_[inner - 1] / diag_[inner - 1];
                for (int i = static_cast<int>(inner) - 2; i >= 0; i--)
                    m_[i + 1] = (rhs_[i] - upper_[i] * m_[i + 2]) / diag_[i];
            }

            // Compute spline coefficients and integrate
            // S_i(x) = a_i + b_i(x-x_i) + c_i(x-x_i)^2 + d_i(x-x_i)^3
            Real price = 0.0;
            for (Size i = 0; i < n_ - 1; i++) {
                Real a_i = values[i];
                Real b_i = (values[i + 1] - values[i]) * h_inv_ -
                            h_ * (m_[i + 1] + 2.0 * m_[i]) / 6.0;
                Real c_i = m_[i] / 2.0;
                Real d_i = (m_[i + 1] - m_[i]) / (6.0 * h_);

                // gaussianShiftedPolynomialIntegral(A4, A3, A2, A1, A0, h, x0, x1)
                // integrates (A4(x-h)^4 + A3(x-h)^3 + A2(x-h)^2 + A1(x-h) + A0) * φ(x)
                // Our spline: d_i(x-z_i)^3 + c_i(x-z_i)^2 + b_i(x-z_i) + a_i
                price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                    0.0, d_i, c_i, b_i, a_i, z_[i], z_[i], z_[i + 1]);
            }

            if (extrapolate) {
                Size last = n_ - 2;
                if (flatExtrapolation) {
                    price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                        0.0, 0.0, 0.0, 0.0, values[last],
                        z_[last], z_[n_ - 1], 100.0);
                    price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                        0.0, 0.0, 0.0, 0.0, values[0],
                        z_[0], -100.0, z_[0]);
                } else {
                    Real d_last = (m_[last + 1] - m_[last]) / (6.0 * h_);
                    Real c_last = m_[last] / 2.0;
                    Real b_last = (values[last + 1] - values[last]) * h_inv_ -
                                   h_ * (m_[last + 1] + 2.0 * m_[last]) / 6.0;
                    Real d_0 = (m_[1] - m_[0]) / (6.0 * h_);
                    Real c_0 = m_[0] / 2.0;
                    Real b_0 = (values[1] - values[0]) * h_inv_ -
                                h_ * (m_[1] + 2.0 * m_[0]) / 6.0;
                    if (isCall)
                        price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                            0.0, d_last, c_last, b_last, values[last],
                            z_[last], z_[n_ - 1], 100.0);
                    if (!isCall)
                        price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                            0.0, d_0, c_0, b_0, values[0],
                            z_[0], -100.0, z_[0]);
                }
            }

            return price;
        }

      private:
        Size n_;
        const Array& z_;
        Real h_, h_inv_, six_h_inv_;
        mutable Array m_;       // second derivatives
        mutable Array rhs_;     // RHS of tridiagonal system
        // Thomas algorithm factorization (stored)
        Array lower_, diag_, upper_;
        // Spline coefficients (reused)
        mutable Array aCoeff_, bCoeff_, cCoeff_;
    };


    Real Gaussian2dNonstandardSwaptionEngine::underlyingNpv(
        const Date& expiry, const Real yRate, const Real ySpread) const {

        Size fixedIdx =
            std::upper_bound(arguments_.fixedResetDates.begin(),
                             arguments_.fixedResetDates.end(), expiry - 1) -
            arguments_.fixedResetDates.begin();
        Size floatingIdx =
            std::upper_bound(arguments_.floatingResetDates.begin(),
                             arguments_.floatingResetDates.end(), expiry - 1) -
            arguments_.floatingResetDates.begin();

        Real type = static_cast<Real>(arguments_.type);
        Real npv = 0.0;

        for (Size i = fixedIdx; i < arguments_.fixedResetDates.size(); i++) {
            npv -= arguments_.fixedCoupons[i] *
                   model_->discountZerobond(arguments_.fixedPayDates[i],
                                            expiry, yRate, ySpread);
        }

        for (Size i = floatingIdx; i < arguments_.floatingResetDates.size(); i++) {
            Real amount;
            if (!arguments_.floatingIsRedemptionFlow[i]) {
                Real fwdRate = model_->forwardRate(
                    arguments_.floatingFixingDates[i], expiry, yRate,
                    arguments_.swap->iborIndex());

                Real couponRate = arguments_.floatingGearings[i] * fwdRate +
                                  arguments_.floatingSpreads[i];

                if (!arguments_.floatingCaps.empty() &&
                    arguments_.floatingCaps[i] != Null<Real>()) {
                    couponRate = std::min(couponRate, arguments_.floatingCaps[i]);
                }
                if (!arguments_.floatingFloors.empty() &&
                    arguments_.floatingFloors[i] != Null<Real>()) {
                    couponRate = std::max(couponRate, arguments_.floatingFloors[i]);
                }

                amount = couponRate *
                         arguments_.floatingAccrualTimes[i] *
                         arguments_.floatingNominal[i];
            } else {
                amount = arguments_.floatingCoupons[i];
            }

            npv += amount *
                   model_->discountZerobond(arguments_.floatingPayDates[i],
                                            expiry, yRate, ySpread);
        }

        return type * npv;
    }

    Real Gaussian2dNonstandardSwaptionEngine::gaussianIntegral1d(
        const Array& z, const Array& values, bool isCall) const {

        // Fallback for external callers — not used in the hot path
        CubicInterpolation payoff(
            z.begin(), z.end(), values.begin(),
            CubicInterpolation::Spline, true,
            CubicInterpolation::Lagrange, 0.0,
            CubicInterpolation::Lagrange, 0.0);

        Real price = 0.0;
        for (Size i = 0; i < z.size() - 1; i++) {
            price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                0.0, payoff.cCoefficients()[i],
                payoff.bCoefficients()[i],
                payoff.aCoefficients()[i],
                values[i], z[i], z[i], z[i + 1]);
        }

        if (extrapolatePayoff_) {
            if (flatPayoffExtrapolation_) {
                price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                    0.0, 0.0, 0.0, 0.0, values[z.size() - 2],
                    z[z.size() - 2], z[z.size() - 1], 100.0);
                price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                    0.0, 0.0, 0.0, 0.0, values[0],
                    z[0], -100.0, z[0]);
            } else {
                if (isCall)
                    price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                        0.0,
                        payoff.cCoefficients()[z.size() - 2],
                        payoff.bCoefficients()[z.size() - 2],
                        payoff.aCoefficients()[z.size() - 2],
                        values[z.size() - 2], z[z.size() - 2],
                        z[z.size() - 1], 100.0);
                if (!isCall)
                    price += Gaussian2dModel::gaussianShiftedPolynomialIntegral(
                        0.0,
                        payoff.cCoefficients()[0],
                        payoff.bCoefficients()[0],
                        payoff.aCoefficients()[0],
                        values[0], z[0], -100.0, z[0]);
            }
        }

        return price;
    }

    void Gaussian2dNonstandardSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "Gaussian2dNonstandardSwaptionEngine");

        Date settlement = model_->termStructure()->referenceDate();

        if (arguments_.exercise->dates().back() <= settlement) {
            results_.value = 0.0;
            return;
        }

        ext::shared_ptr<RebatedExercise> rebatedExercise =
            ext::dynamic_pointer_cast<RebatedExercise>(arguments_.exercise);

        int idx = arguments_.exercise->dates().size() - 1;
        int minIdxAlive = static_cast<int>(
            std::upper_bound(arguments_.exercise->dates().begin(),
                             arguments_.exercise->dates().end(), settlement) -
            arguments_.exercise->dates().begin());

        Option::Type type =
            arguments_.type == Swap::Payer ? Option::Call : Option::Put;
        bool isCall = (type == Option::Call);

        Size N = 2 * integrationPoints_ + 1;

        // Standardized grid (fixed, uniform)
        Array z = model_->yGrid(stddevs_, integrationPoints_);

        // Create SplineIntegrator once — reused for all Gaussian integrals.
        // Pre-factors the tridiagonal LU decomposition on the z grid.
        SplineIntegrator integrator(z);

        // 2D NPV arrays: npv[kr * N + ks]
        std::vector<Real> npv0(N * N, 0.0);
        std::vector<Real> npv1(N * N, 0.0);

        Date expiry1 = Date();
        Time expiry1Time = Null<Real>(), expiry0Time;

        // Reusable working arrays (avoid repeated allocation)
        Array outerVals(N), innerVals(N);

        do {
            Date expiry0;
            if (idx == minIdxAlive - 1)
                expiry0 = settlement;
            else
                expiry0 = arguments_.exercise->dates()[idx];

            expiry0Time = std::max(
                model_->termStructure()->timeFromReference(expiry0), 0.0);

            Real rho = model_->correlation(expiry0Time);
            Real sqrtOneMinusRhoSq = std::sqrt(std::max(1.0 - rho * rho, 0.0));

            Size activeN = (expiry0 > settlement ? N : 1);

            if (expiry1Time != Null<Real>()) {
                // ===========================================================
                // PRECOMPUTE (once per exercise date):
                //   1. Rate-direction cubic interpolations
                //   2. Spread grid slope
                //   3. Spread grid centers for ALL ks values
                // ===========================================================

                // 1. Rate interps: one per spread grid column
                std::vector<Array> rateColumns(N, Array(N));
                std::vector<CubicInterpolation> rateInterps;
                rateInterps.reserve(N);

                for (Size j_s = 0; j_s < N; j_s++) {
                    for (Size ir = 0; ir < N; ir++)
                        rateColumns[j_s][ir] = npv1[ir * N + j_s];

                    rateInterps.emplace_back(
                        z.begin(), z.end(), rateColumns[j_s].begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                }

                // 2. Spread grid slope (constant for all ks)
                Array yg_s_ref = model_->yGrid(
                    model_->spreadProcess(), stddevs_,
                    integrationPoints_, expiry1Time, expiry0Time, 0.0);
                Real slope_s = (integrationPoints_ > 0)
                    ? (yg_s_ref[integrationPoints_ + 1] - yg_s_ref[integrationPoints_]) /
                      (z[integrationPoints_ + 1] - z[integrationPoints_])
                    : 1.0;

                // 3. Spread grid centers for all ks (linear in z[ks])
                //    center_s(ks) = yg_s_ref[mid] + slope_s * (z_ks_shift)
                //    The yGrid is linear: yg[j] = center(y) + slope * z[j]
                //    where center depends linearly on the conditioning state y.
                //    So center_s[ks] = center_at_0 + d_center * z[ks]
                Real center_s_at_0 = yg_s_ref[integrationPoints_]; // center when ks=mid (z=0)

                // Compute center at z[0] and z[N-1] to get the linear relationship
                Array yg_s_lo = model_->yGrid(
                    model_->spreadProcess(), stddevs_,
                    integrationPoints_, expiry1Time, expiry0Time, z[0]);
                Real center_s_lo = yg_s_lo[integrationPoints_];
                Real d_center_dz = (z[0] != 0.0)
                    ? (center_s_lo - center_s_at_0) / z[0]
                    : 0.0;

                // Precompute all centers
                std::vector<Real> centerS(N);
                for (Size ks = 0; ks < N; ks++)
                    centerS[ks] = center_s_at_0 + d_center_dz * z[ks];

                for (Size kr = 0; kr < activeN; kr++) {

                    // Rate conditional grid at t1 given z[kr] at t0
                    Array yg_r = model_->yGrid(
                        model_->rateProcess(), stddevs_,
                        integrationPoints_, expiry1Time, expiry0Time,
                        expiry0 > settlement ? z[kr] : 0.0);

                    // Evaluate all rate interps at this kr's yg_r points.
                    // Then build spread-direction interps for each outer i.
                    std::vector<Array> rateEvals(N, Array(N));
                    std::vector<CubicInterpolation> spreadInterps;
                    spreadInterps.reserve(N);

                    for (Size i = 0; i < N; i++) {
                        for (Size j_s = 0; j_s < N; j_s++)
                            rateEvals[i][j_s] = rateInterps[j_s](yg_r[i], true);

                        spreadInterps.emplace_back(
                            z.begin(), z.end(), rateEvals[i].begin(),
                            CubicInterpolation::Spline, true,
                            CubicInterpolation::Lagrange, 0.0,
                            CubicInterpolation::Lagrange, 0.0);
                    }

                    for (Size ks = 0; ks < activeN; ks++) {

                        Real center_s = centerS[ks];

                        for (Size i = 0; i < N; i++) {
                            for (Size j = 0; j < N; j++) {
                                Real effectiveInnovation =
                                    rho * z[i] + sqrtOneMinusRhoSq * z[j];
                                Real yg_s_eff =
                                    center_s + slope_s * effectiveInnovation;

                                innerVals[j] = spreadInterps[i](yg_s_eff, true);
                            }

                            outerVals[i] = integrator.integrate(
                                innerVals, isCall,
                                extrapolatePayoff_, flatPayoffExtrapolation_);
                        }

                        Real price = integrator.integrate(
                            outerVals, isCall,
                            extrapolatePayoff_, flatPayoffExtrapolation_);

                        npv0[kr * N + ks] = price;

                        if (expiry0 > settlement) {
                            Real exerciseValue =
                                underlyingNpv(expiry0, z[kr], z[ks]) /
                                model_->numeraire(expiry0Time, z[kr], z[ks]);

                            if (rebatedExercise != nullptr) {
                                Real rebate = rebatedExercise->rebate(idx);
                                Date rebateDate =
                                    rebatedExercise->rebatePaymentDate(idx);
                                exerciseValue +=
                                    rebate *
                                    model_->discountZerobond(
                                        rebateDate, expiry0, z[kr], z[ks]) /
                                    model_->numeraire(expiry0Time, z[kr], z[ks]);
                            }

                            npv0[kr * N + ks] =
                                std::max(npv0[kr * N + ks], exerciseValue);
                        }
                    }
                }
            } else {
                // First iteration (no backward step)
                for (Size kr = 0; kr < activeN; kr++) {
                    for (Size ks = 0; ks < activeN; ks++) {
                        npv0[kr * N + ks] = 0.0;
                        if (expiry0 > settlement) {
                            Real exerciseValue =
                                underlyingNpv(expiry0, z[kr], z[ks]) /
                                model_->numeraire(expiry0Time, z[kr], z[ks]);

                            if (rebatedExercise != nullptr) {
                                Real rebate = rebatedExercise->rebate(idx);
                                Date rebateDate =
                                    rebatedExercise->rebatePaymentDate(idx);
                                exerciseValue +=
                                    rebate *
                                    model_->discountZerobond(
                                        rebateDate, expiry0, z[kr], z[ks]) /
                                    model_->numeraire(expiry0Time, z[kr], z[ks]);
                            }

                            npv0[kr * N + ks] = exerciseValue;
                        }
                    }
                }
            }

            npv0.swap(npv1);

            expiry1 = expiry0;
            expiry1Time = expiry0Time;

        } while (--idx >= minIdxAlive - 1);

        results_.value = npv1[0] * model_->numeraire(0.0, 0.0, 0.0);
    }
}
