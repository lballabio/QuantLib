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
    // ================================================================

    class SplineIntegrator {
      public:
        explicit SplineIntegrator(const Array& z)
            : n_(z.size()), z_(z),
              m_(n_, 0.0), rhs_(n_, 0.0),
              lower_(n_, 0.0), diag_(n_, 0.0), upper_(n_, 0.0) {

            QL_REQUIRE(n_ >= 3, "need at least 3 grid points");
            h_ = z_[1] - z_[0];
            h_inv_ = 1.0 / h_;
            six_h_inv_ = 6.0 * h_inv_;

            Size inner = n_ - 2;
            if (inner == 0) return;

            for (Size i = 0; i < inner; i++) {
                diag_[i] = 4.0 * h_;
                lower_[i] = h_;
                upper_[i] = h_;
            }

            for (Size i = 1; i < inner; i++) {
                Real w = lower_[i] / diag_[i - 1];
                diag_[i] -= w * upper_[i - 1];
                lower_[i] = w;
            }
        }

        Real integrate(const Array& values, bool isCall,
                       bool extrapolate, bool flatExtrapolation) const {

            Size inner = n_ - 2;

            for (Size i = 0; i < inner; i++)
                rhs_[i] = six_h_inv_ * (values[i + 2] - 2.0 * values[i + 1] + values[i]);

            for (Size i = 1; i < inner; i++)
                rhs_[i] -= lower_[i] * rhs_[i - 1];

            m_[0] = 0.0;
            m_[n_ - 1] = 0.0;
            if (inner > 0) {
                m_[inner] = rhs_[inner - 1] / diag_[inner - 1];
                for (int i = static_cast<int>(inner) - 2; i >= 0; i--)
                    m_[i + 1] = (rhs_[i] - upper_[i] * m_[i + 2]) / diag_[i];
            }

            Real price = 0.0;
            for (Size i = 0; i < n_ - 1; i++) {
                Real a_i = values[i];
                Real b_i = (values[i + 1] - values[i]) * h_inv_ -
                            h_ * (m_[i + 1] + 2.0 * m_[i]) / 6.0;
                Real c_i = m_[i] / 2.0;
                Real d_i = (m_[i + 1] - m_[i]) / (6.0 * h_);

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
        mutable Array m_, rhs_;
        Array lower_, diag_, upper_;
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
        Real typeSign = static_cast<Real>(arguments_.type);

        Size N = 2 * integrationPoints_ + 1;

        Array z = model_->yGrid(stddevs_, integrationPoints_);
        SplineIntegrator integrator(z);

        std::vector<Real> npv0(N * N, 0.0);
        std::vector<Real> npv1(N * N, 0.0);

        Date expiry1 = Date();
        Time expiry1Time = Null<Real>(), expiry0Time;

        // Reusable working arrays (pre-allocated once)
        Array outerVals(N), innerVals(N);
        std::vector<Array> rateEvals(N, Array(N));

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

            // ===========================================================
            // PRECOMPUTE EXERCISE VALUES: Factor discountZerobond as
            //   discountZB(p, t, yr, ys) = zbR0(yr, p) * zb0S(ys, p) / zb00(p)
            //
            // This holds for affine Gaussian models where the two factors
            // contribute multiplicatively. It reduces N² × nCoupon model
            // calls to 2 × N × nCoupon (one sweep per factor).
            //
            // Forward rates depend only on yr and are also precomputed.
            // ===========================================================

            Size fixedIdx = 0, floatingIdx = 0;
            if (expiry0 > settlement) {
                fixedIdx =
                    std::upper_bound(arguments_.fixedResetDates.begin(),
                                     arguments_.fixedResetDates.end(), expiry0 - 1) -
                    arguments_.fixedResetDates.begin();
                floatingIdx =
                    std::upper_bound(arguments_.floatingResetDates.begin(),
                                     arguments_.floatingResetDates.end(), expiry0 - 1) -
                    arguments_.floatingResetDates.begin();
            }

            Size nFixed = arguments_.fixedResetDates.size() - fixedIdx;
            Size nFloat = arguments_.floatingResetDates.size() - floatingIdx;

            // Discount zerobond tables: zbR0[kr][p], zb0S[ks][p], zb00[p]
            // p indexes over all relevant pay dates (fixed + floating)
            std::vector<std::vector<Real>> zbR0, zb0S;
            std::vector<Real> zb00;
            // Forward rate table: fwdR[kr][f]
            std::vector<std::vector<Real>> fwdR;
            // Numeraire factors: numR0[kr], num0S[ks], num00
            std::vector<Real> numR0(activeN), num0S(activeN);
            Real num00 = 1.0;

            if (expiry0 > settlement) {
                Size nPay = nFixed + nFloat;
                zb00.resize(nPay);
                zbR0.resize(activeN, std::vector<Real>(nPay));
                zb0S.resize(activeN, std::vector<Real>(nPay));
                fwdR.resize(activeN, std::vector<Real>(nFloat));

                // zb00: discountZerobond at (yr=0, ys=0) for each pay date
                for (Size p = 0; p < nFixed; p++)
                    zb00[p] = model_->discountZerobond(
                        arguments_.fixedPayDates[fixedIdx + p], expiry0, 0.0, 0.0);
                for (Size p = 0; p < nFloat; p++)
                    zb00[nFixed + p] = model_->discountZerobond(
                        arguments_.floatingPayDates[floatingIdx + p], expiry0, 0.0, 0.0);

                // Numeraire at (0, 0)
                num00 = model_->numeraire(expiry0Time, 0.0, 0.0);

                for (Size k = 0; k < activeN; k++) {
                    // Rate-factor sweep: zbR0[k][p] = discountZB(p, expiry, z[k], 0)
                    for (Size p = 0; p < nFixed; p++)
                        zbR0[k][p] = model_->discountZerobond(
                            arguments_.fixedPayDates[fixedIdx + p], expiry0, z[k], 0.0);
                    for (Size p = 0; p < nFloat; p++)
                        zbR0[k][nFixed + p] = model_->discountZerobond(
                            arguments_.floatingPayDates[floatingIdx + p], expiry0, z[k], 0.0);

                    // Spread-factor sweep: zb0S[k][p] = discountZB(p, expiry, 0, z[k])
                    for (Size p = 0; p < nFixed; p++)
                        zb0S[k][p] = model_->discountZerobond(
                            arguments_.fixedPayDates[fixedIdx + p], expiry0, 0.0, z[k]);
                    for (Size p = 0; p < nFloat; p++)
                        zb0S[k][nFixed + p] = model_->discountZerobond(
                            arguments_.floatingPayDates[floatingIdx + p], expiry0, 0.0, z[k]);

                    // Forward rates (rate factor only)
                    for (Size f = 0; f < nFloat; f++) {
                        if (!arguments_.floatingIsRedemptionFlow[floatingIdx + f])
                            fwdR[k][f] = model_->forwardRate(
                                arguments_.floatingFixingDates[floatingIdx + f],
                                expiry0, z[k], arguments_.swap->iborIndex());
                        else
                            fwdR[k][f] = 0.0;
                    }

                    // Numeraire factors
                    numR0[k] = model_->numeraire(expiry0Time, z[k], 0.0);
                    num0S[k] = model_->numeraire(expiry0Time, 0.0, z[k]);
                }
            }

            if (expiry1Time != Null<Real>()) {
                // ===========================================================
                // BACKWARD INDUCTION
                // ===========================================================

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

                Array yg_s_ref = model_->yGrid(
                    model_->spreadProcess(), stddevs_,
                    integrationPoints_, expiry1Time, expiry0Time, 0.0);
                Real slope_s = (integrationPoints_ > 0)
                    ? (yg_s_ref[integrationPoints_ + 1] - yg_s_ref[integrationPoints_]) /
                      (z[integrationPoints_ + 1] - z[integrationPoints_])
                    : 1.0;

                Real center_s_at_0 = yg_s_ref[integrationPoints_];
                Array yg_s_lo = model_->yGrid(
                    model_->spreadProcess(), stddevs_,
                    integrationPoints_, expiry1Time, expiry0Time, z[0]);
                Real d_center_dz = (z[0] != 0.0)
                    ? (yg_s_lo[integrationPoints_] - center_s_at_0) / z[0]
                    : 0.0;

                std::vector<Real> centerS(N);
                for (Size ks = 0; ks < N; ks++)
                    centerS[ks] = center_s_at_0 + d_center_dz * z[ks];

                for (Size kr = 0; kr < activeN; kr++) {

                    Array yg_r = model_->yGrid(
                        model_->rateProcess(), stddevs_,
                        integrationPoints_, expiry1Time, expiry0Time,
                        expiry0 > settlement ? z[kr] : 0.0);

                    // Evaluate rate interps at yg_r points → reuse rateEvals
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
                                Real eff = rho * z[i] + sqrtOneMinusRhoSq * z[j];
                                innerVals[j] = spreadInterps[i](
                                    center_s + slope_s * eff, true);
                            }

                            outerVals[i] = integrator.integrate(
                                innerVals, isCall,
                                extrapolatePayoff_, flatPayoffExtrapolation_);
                        }

                        Real price = integrator.integrate(
                            outerVals, isCall,
                            extrapolatePayoff_, flatPayoffExtrapolation_);

                        npv0[kr * N + ks] = price;

                        // Exercise decision using precomputed tables
                        if (expiry0 > settlement) {
                            Real invNum = num00 / (numR0[kr] * num0S[ks]);

                            // Inline underlyingNpv using factored zerobonds
                            Real swapNpv = 0.0;
                            for (Size p = 0; p < nFixed; p++) {
                                Real zb = zbR0[kr][p] * zb0S[ks][p] / zb00[p];
                                swapNpv -= arguments_.fixedCoupons[fixedIdx + p] * zb;
                            }
                            for (Size f = 0; f < nFloat; f++) {
                                Size fi = floatingIdx + f;
                                Real amount;
                                if (!arguments_.floatingIsRedemptionFlow[fi]) {
                                    Real couponRate =
                                        arguments_.floatingGearings[fi] * fwdR[kr][f] +
                                        arguments_.floatingSpreads[fi];
                                    if (!arguments_.floatingCaps.empty() &&
                                        arguments_.floatingCaps[fi] != Null<Real>())
                                        couponRate = std::min(couponRate,
                                                              arguments_.floatingCaps[fi]);
                                    if (!arguments_.floatingFloors.empty() &&
                                        arguments_.floatingFloors[fi] != Null<Real>())
                                        couponRate = std::max(couponRate,
                                                              arguments_.floatingFloors[fi]);
                                    amount = couponRate *
                                             arguments_.floatingAccrualTimes[fi] *
                                             arguments_.floatingNominal[fi];
                                } else {
                                    amount = arguments_.floatingCoupons[fi];
                                }
                                Real zb = zbR0[kr][nFixed + f] * zb0S[ks][nFixed + f] /
                                          zb00[nFixed + f];
                                swapNpv += amount * zb;
                            }

                            Real exerciseValue = typeSign * swapNpv * invNum;

                            if (rebatedExercise != nullptr) {
                                Real rebate = rebatedExercise->rebate(idx);
                                Date rebateDate =
                                    rebatedExercise->rebatePaymentDate(idx);
                                exerciseValue +=
                                    rebate *
                                    model_->discountZerobond(
                                        rebateDate, expiry0, z[kr], z[ks]) *
                                    invNum;
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
                            Real invNum = num00 / (numR0[kr] * num0S[ks]);

                            Real swapNpv = 0.0;
                            for (Size p = 0; p < nFixed; p++) {
                                Real zb = zbR0[kr][p] * zb0S[ks][p] / zb00[p];
                                swapNpv -= arguments_.fixedCoupons[fixedIdx + p] * zb;
                            }
                            for (Size f = 0; f < nFloat; f++) {
                                Size fi = floatingIdx + f;
                                Real amount;
                                if (!arguments_.floatingIsRedemptionFlow[fi]) {
                                    Real couponRate =
                                        arguments_.floatingGearings[fi] * fwdR[kr][f] +
                                        arguments_.floatingSpreads[fi];
                                    if (!arguments_.floatingCaps.empty() &&
                                        arguments_.floatingCaps[fi] != Null<Real>())
                                        couponRate = std::min(couponRate,
                                                              arguments_.floatingCaps[fi]);
                                    if (!arguments_.floatingFloors.empty() &&
                                        arguments_.floatingFloors[fi] != Null<Real>())
                                        couponRate = std::max(couponRate,
                                                              arguments_.floatingFloors[fi]);
                                    amount = couponRate *
                                             arguments_.floatingAccrualTimes[fi] *
                                             arguments_.floatingNominal[fi];
                                } else {
                                    amount = arguments_.floatingCoupons[fi];
                                }
                                Real zb = zbR0[kr][nFixed + f] * zb0S[ks][nFixed + f] /
                                          zb00[nFixed + f];
                                swapNpv += amount * zb;
                            }

                            npv0[kr * N + ks] = typeSign * swapNpv * invNum;

                            if (rebatedExercise != nullptr) {
                                Real rebate = rebatedExercise->rebate(idx);
                                Date rebateDate =
                                    rebatedExercise->rebatePaymentDate(idx);
                                npv0[kr * N + ks] =
                                    rebate *
                                    model_->discountZerobond(
                                        rebateDate, expiry0, z[kr], z[ks]) *
                                    invNum;
                            }
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
