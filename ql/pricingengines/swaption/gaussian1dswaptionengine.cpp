/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    void Gaussian1dSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "Gaussian1dSwaptionEngine");

        Date settlement = model_->termStructure()->referenceDate();

        if (arguments_.exercise->dates().back() <=
            settlement) { // swaption is expired, possibly generated swap is not
                          // valued
            results_.value = 0.0;
            return;
        }

        int idx = static_cast<int>(arguments_.exercise->dates().size()) - 1;
        int minIdxAlive = static_cast<int>(
            std::upper_bound(arguments_.exercise->dates().begin(),
                             arguments_.exercise->dates().end(), settlement) -
            arguments_.exercise->dates().begin());

        VanillaSwap swap = *arguments_.swap;
        Option::Type type =
            arguments_.type == Swap::Payer ? Option::Call : Option::Put;
        const Schedule& fixedSchedule = swap.fixedSchedule();
        const Schedule& floatSchedule = swap.floatingSchedule();

        Array npv0(2 * integrationPoints_ + 1, 0.0),
            npv1(2 * integrationPoints_ + 1, 0.0);
        Array z = model_->yGrid(stddevs_, integrationPoints_);
        Array p(z.size(), 0.0);

        // for probability computation
        std::vector<Array> npvp0, npvp1;
        if (probabilities_ != None) {
            for (int i = 0; i < idx - minIdxAlive + 2; ++i) {
                Array npvTmp0(2 * integrationPoints_ + 1, 0.0);
                Array npvTmp1(2 * integrationPoints_ + 1, 0.0);
                npvp0.push_back(npvTmp0);
                npvp1.push_back(npvTmp1);
            }
        }
        // end probabkility computation

        Date expiry1 = Null<Date>(), expiry0;
        Time expiry1Time = Null<Real>(), expiry0Time;

        do {

            if (idx == minIdxAlive - 1)
                expiry0 = settlement;
            else
                expiry0 = arguments_.exercise->dates()[idx];

            expiry0Time = std::max(
                model_->termStructure()->timeFromReference(expiry0), 0.0);

            Size j1 =
                std::upper_bound(fixedSchedule.dates().begin(),
                                 fixedSchedule.dates().end(), expiry0 - 1) -
                fixedSchedule.dates().begin();
            Size k1 =
                std::upper_bound(floatSchedule.dates().begin(),
                                 floatSchedule.dates().end(), expiry0 - 1) -
                floatSchedule.dates().begin();

            // a lazy object is not thread safe, neither is the caching
            // in gsrprocess. therefore we trigger computations here such
            // that neither lazy object recalculation nor write access
            // during caching occurs in the parallized loop below.
            // this is known to work for the gsr and markov functional
            // model implementations of Gaussian1dModel
#ifdef _OPENMP
            if (expiry1Time != Null<Real>())
                model_->yGrid(stddevs_, integrationPoints_, expiry1Time,
                              expiry0Time, 0.0);
            if (expiry0 > settlement) {
                for (Size l = k1; l < arguments_.floatingCoupons.size(); l++) {
                    model_->forwardRate(arguments_.floatingFixingDates[l],
                                        expiry0, 0.0,
                                        arguments_.swap->iborIndex());
                    model_->zerobond(arguments_.floatingPayDates[l], expiry0,
                                     0.0, discountCurve_);
                }
                for (Size l = j1; l < arguments_.fixedCoupons.size(); l++) {
                    model_->zerobond(arguments_.fixedPayDates[l], expiry0, 0.0,
                                     discountCurve_);
                }
                model_->numeraire(expiry0Time, 0.0, discountCurve_);
            }
#endif

#pragma omp parallel for default(shared) firstprivate(p) if(expiry0>settlement)
            for (long k = 0; k < (expiry0 > settlement ? (long)npv0.size() : 1);
                 k++) {

                Real price = 0.0;
                if (expiry1Time != Null<Real>()) {
                    Array yg = model_->yGrid(stddevs_, integrationPoints_,
                                             expiry1Time, expiry0Time,
                                             expiry0 > settlement ? z[k] : 0.0);
                    CubicInterpolation payoff0(
                        z.begin(), z.end(), npv1.begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                    for (Size i = 0; i < yg.size(); i++) {
                        p[i] = payoff0(yg[i], true);
                    }
                    CubicInterpolation payoff1(
                        z.begin(), z.end(), p.begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                    for (Size i = 0; i < z.size() - 1; i++) {
                        price += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                            0.0, payoff1.cCoefficients()[i],
                            payoff1.bCoefficients()[i],
                            payoff1.aCoefficients()[i], p[i], z[i], z[i],
                            z[i + 1]);
                    }
                    if (extrapolatePayoff_) {
                        if (flatPayoffExtrapolation_) {
                            price += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                0.0, 0.0, 0.0, 0.0, p[z.size() - 2],
                                z[z.size() - 2], z[z.size() - 1], 100.0);
                            price += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0, z[0]);
                        } else {
                            if (type == Option::Call)
                                price +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0,
                                        payoff1.cCoefficients()[z.size() - 2],
                                        payoff1.bCoefficients()[z.size() - 2],
                                        payoff1.aCoefficients()[z.size() - 2],
                                        p[z.size() - 2], z[z.size() - 2],
                                        z[z.size() - 1], 100.0);
                            if (type == Option::Put)
                                price +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0, payoff1.cCoefficients()[0],
                                        payoff1.bCoefficients()[0],
                                        payoff1.aCoefficients()[0], p[0], z[0],
                                        -100.0, z[0]);
                        }
                    }
                }

                npv0[k] = price;

                // for probability computation
                if (probabilities_ != None) {
                    for (Size m = 0; m < npvp0.size(); m++) {
                        Real price = 0.0;
                        if (expiry1Time != Null<Real>()) {
                            Array yg = model_->yGrid(
                                stddevs_, integrationPoints_, expiry1Time,
                                expiry0Time, expiry0 > settlement ? z[k] : 0.0);
                            CubicInterpolation payoff0(
                                z.begin(), z.end(), npvp1[m].begin(),
                                CubicInterpolation::Spline, true,
                                CubicInterpolation::Lagrange, 0.0,
                                CubicInterpolation::Lagrange, 0.0);
                            for (Size i = 0; i < yg.size(); i++) {
                                p[i] = payoff0(yg[i], true);
                            }
                            CubicInterpolation payoff1(
                                z.begin(), z.end(), p.begin(),
                                CubicInterpolation::Spline, true,
                                CubicInterpolation::Lagrange, 0.0,
                                CubicInterpolation::Lagrange, 0.0);
                            for (Size i = 0; i < z.size() - 1; i++) {
                                price +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0, payoff1.cCoefficients()[i],
                                        payoff1.bCoefficients()[i],
                                        payoff1.aCoefficients()[i], p[i], z[i],
                                        z[i], z[i + 1]);
                            }
                            if (extrapolatePayoff_) {
                                if (flatPayoffExtrapolation_) {
                                    price +=
                                        Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                                  0.0, 0.0, 0.0, 0.0,
                                                  p[z.size() - 2],
                                                  z[z.size() - 2],
                                                  z[z.size() - 1], 100.0);
                                    price +=
                                        Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                                  0.0, 0.0, 0.0, 0.0, p[0],
                                                  z[0], -100.0, z[0]);
                                } else {
                                    if (type == Option::Call)
                                        price +=
                                            Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                                      0.0,
                                                      payoff1.cCoefficients()
                                                          [z.size() - 2],
                                                      payoff1.bCoefficients()
                                                          [z.size() - 2],
                                                      payoff1.aCoefficients()
                                                          [z.size() - 2],
                                                      p[z.size() - 2],
                                                      z[z.size() - 2],
                                                      z[z.size() - 1], 100.0);
                                    if (type == Option::Put)
                                        price +=
                                            Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                                      0.0,
                                                      payoff1
                                                          .cCoefficients()[0],
                                                      payoff1
                                                          .bCoefficients()[0],
                                                      payoff1
                                                          .aCoefficients()[0],
                                                      p[0], z[0], -100.0, z[0]);
                                }
                            }
                        }

                        npvp0[m][k] = price;
                    }
                }
                // end probability computation

                if (expiry0 > settlement) {
                    Real floatingLegNpv = 0.0;
                    for (Size l = k1; l < arguments_.floatingCoupons.size();
                         l++) {
                        floatingLegNpv +=
                            arguments_.nominal *
                            arguments_.floatingAccrualTimes[l] *
                            (arguments_.floatingSpreads[l] +
                             model_->forwardRate(
                                 arguments_.floatingFixingDates[l], expiry0,
                                 z[k], arguments_.swap->iborIndex())) *
                            model_->zerobond(arguments_.floatingPayDates[l],
                                             expiry0, z[k], discountCurve_);
                    }
                    Real fixedLegNpv = 0.0;
                    for (Size l = j1; l < arguments_.fixedCoupons.size(); l++) {
                        fixedLegNpv +=
                            arguments_.fixedCoupons[l] *
                            model_->zerobond(arguments_.fixedPayDates[l],
                                             expiry0, z[k], discountCurve_);
                    }
                    Real exerciseValue =
                        (type == Option::Call ? 1.0 : -1.0) *
                        (floatingLegNpv - fixedLegNpv) /
                        model_->numeraire(expiry0Time, z[k], discountCurve_);

                    // for probability computation
                    if (probabilities_ != None) {
                        if (idx == static_cast<int>(
                                       arguments_.exercise->dates().size()) -
                                       1) // if true we are at the latest date,
                                          // so we init
                                          // the no call probability
                            npvp0.back()[k] =
                                probabilities_ == Naive
                                    ? 1.0
                                    : 1.0 / (model_->zerobond(expiry0Time, 0.0,
                                                              0.0,
                                                              discountCurve_) *
                                             model_->numeraire(expiry0, z[k],
                                                               discountCurve_));
                        if (exerciseValue >= npv0[k]) {
                            npvp0[idx - minIdxAlive][k] =
                                probabilities_ == Naive
                                    ? 1.0
                                    : 1.0 /
                                          (model_->zerobond(expiry0Time, 0.0,
                                                            0.0,
                                                            discountCurve_) *
                                           model_->numeraire(expiry0Time, z[k],
                                                             discountCurve_));
                            for (Size ii = idx - minIdxAlive + 1;
                                 ii < npvp0.size(); ii++)
                                npvp0[ii][k] = 0.0;
                        }
                    }
                    // end probability computation

                    npv0[k] = std::max(npv0[k], exerciseValue);
                }
            }

            npv1.swap(npv0);

            // for probability computation
            if (probabilities_ != None) {
                for (Size i = 0; i < npvp0.size(); i++)
                    npvp1[i].swap(npvp0[i]);
            }
            // end probability computation

            expiry1 = expiry0;
            expiry1Time = expiry0Time;

        } while (--idx >= minIdxAlive - 1);

        results_.value = npv1[0] * model_->numeraire(0.0, 0.0, discountCurve_);

        // for probability computation
        if (probabilities_ != None) {
            std::vector<Real> prob(npvp0.size());
            for (Size i = 0; i < npvp0.size(); i++) {
                prob[i] = npvp1[i][0] *
                          (probabilities_ == Naive
                               ? 1.0
                               : model_->numeraire(0.0, 0.0, discountCurve_));
            }
            results_.additionalResults["probabilities"] = prob;
        }
        // end probability computation
    }
}
