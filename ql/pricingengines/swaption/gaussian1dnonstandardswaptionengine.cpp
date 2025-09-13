/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/pricingengines/swaption/gaussian1dnonstandardswaptionengine.hpp>
#include <ql/rebatedexercise.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/payoff.hpp>

using std::exp;

namespace QuantLib {

    Real
    Gaussian1dNonstandardSwaptionEngine::underlyingNpv(const Date &expiry,
                                                       const Real y) const {

        // determine the indices on both legs representing the cashflows that
        // are part of the exercise into right

        Size fixedIdx =
            std::upper_bound(arguments_.fixedResetDates.begin(),
                             arguments_.fixedResetDates.end(), expiry - 1) -
            arguments_.fixedResetDates.begin();
        Size floatingIdx =
            std::upper_bound(arguments_.floatingResetDates.begin(),
                             arguments_.floatingResetDates.end(), expiry - 1) -
            arguments_.floatingResetDates.begin();

        // calculate the npv of these cashflows conditional on y at expiry

        Real type = (Real)arguments_.type;

        Real npv = 0.0;
        for (Size i = fixedIdx; i < arguments_.fixedResetDates.size(); i++) {
            npv -=
                arguments_.fixedCoupons[i] *
                model_->zerobond(arguments_.fixedPayDates[i], expiry, y,
                                 discountCurve_) *
                (oas_.empty()
                     ? Real(1.0)
                     : exp(-oas_->value() *
                           model_->termStructure()->dayCounter().yearFraction(
                               expiry, arguments_.fixedPayDates[i])));
        }

        for (Size i = floatingIdx; i < arguments_.floatingResetDates.size();
             i++) {
            Real amount;
            if (!arguments_.floatingIsRedemptionFlow[i])
                amount = (arguments_.floatingGearings[i] *
                              model_->forwardRate(
                                  arguments_.floatingFixingDates[i], expiry, y,
                                  arguments_.swap->iborIndex()) +
                          arguments_.floatingSpreads[i]) *
                         arguments_.floatingAccrualTimes[i] *
                         arguments_.floatingNominal[i];
            else
                amount = arguments_.floatingCoupons[i];
            npv +=
                amount * model_->zerobond(arguments_.floatingPayDates[i],
                                          expiry, y, discountCurve_) *
                (oas_.empty()
                     ? Real(1.0)
                     : exp(-oas_->value() *
                           model_->termStructure()->dayCounter().yearFraction(
                               expiry, arguments_.floatingPayDates[i])));
        }

        return type * npv;
    }

    Swap::Type Gaussian1dNonstandardSwaptionEngine::underlyingType() const {
        return arguments_.swap->type();
    }

    // NOLINTNEXTLINE(readability-const-return-type)
    const Date Gaussian1dNonstandardSwaptionEngine::underlyingLastDate() const {
        return arguments_.fixedPayDates.back();
    }

    // NOLINTNEXTLINE(readability-const-return-type)
    const Array Gaussian1dNonstandardSwaptionEngine::initialGuess(const Date &expiry) const {

        Size fixedIdx =
            std::upper_bound(arguments_.fixedResetDates.begin(),
                             arguments_.fixedResetDates.end(), expiry - 1) -
            arguments_.fixedResetDates.begin();

        Array initial(3);
        Real nominalSum = 0.0, weightedRate = 0.0, ind = 0.0;
        for (Size i = fixedIdx; i < arguments_.fixedResetDates.size(); i++) {
            nominalSum += arguments_.fixedNominal[i];
            Real rate = arguments_.fixedRate[i];
            if (close(rate, 0.0))
                rate = 0.03; // this value is at least better than zero
            weightedRate += arguments_.fixedNominal[i] * rate;
            if (arguments_.fixedNominal[i] > 1E-8) // exclude zero nominal periods
                ind += 1.0;
        }
        Real nominalAvg = nominalSum / ind;

        QL_REQUIRE(nominalSum > 0.0,
                   "sum of nominals on fixed leg must be positive ("
                       << nominalSum << ")");

        weightedRate /= nominalSum;
        initial[0] = nominalAvg;
        initial[1] =
            model_->termStructure()->timeFromReference(underlyingLastDate()) -
            model_->termStructure()->timeFromReference(expiry);
        initial[2] = weightedRate;

        return initial;
    }

    void Gaussian1dNonstandardSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "Gaussian1dNonstandardSwaptionEngine");

        Date settlement = model_->termStructure()->referenceDate();

        if (arguments_.exercise->dates().back() <=
            settlement) { // swaption is expired, possibly generated swap is not
                          // valued
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

        NonstandardSwap swap = *arguments_.swap;
        Option::Type type =
            arguments_.type == Swap::Payer ? Option::Call : Option::Put;

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
        // end probability computation

        Date expiry1 = Date(), expiry0;
        Time expiry1Time = Null<Real>(), expiry0Time;

        do {

            if (idx == minIdxAlive - 1)
                expiry0 = settlement;
            else
                expiry0 = arguments_.exercise->dates()[idx];

            expiry0Time = std::max(
                model_->termStructure()->timeFromReference(expiry0), 0.0);

            Size j1 =
                std::upper_bound(arguments_.fixedResetDates.begin(),
                                 arguments_.fixedResetDates.end(), expiry0 - 1) -
                arguments_.fixedResetDates.begin();
            Size k1 =
                std::upper_bound(arguments_.floatingResetDates.begin(),
                                 arguments_.floatingResetDates.end(), expiry0 - 1) -
                arguments_.floatingResetDates.begin();

            // todo add openmp support later on (as in gaussian1dswaptionengine)

            for (Size k = 0; k < (expiry0 > settlement ? npv0.size() : 1);
                 k++) {

                Real price = 0.0;
                if (expiry1Time != Null<Real>()) {
                    Real zSpreadDf =
                        oas_.empty() ? Real(1.0)
                                     : std::exp(-oas_->value() *
                                                (expiry1Time - expiry0Time));
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
                                     payoff1.aCoefficients()[i], p[i], z[i],
                                     z[i], z[i + 1]) *
                                 zSpreadDf;
                    }
                    if (extrapolatePayoff_) {
                        if (flatPayoffExtrapolation_) {
                            price +=
                                Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                    0.0, 0.0, 0.0, 0.0, p[z.size() - 2],
                                    z[z.size() - 2], z[z.size() - 1], 100.0) *
                                zSpreadDf;
                            price += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                         0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0,
                                         z[0]) *
                                     zSpreadDf;
                        } else {
                            if (type == Option::Call)
                                price +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0,
                                        payoff1.cCoefficients()[z.size() - 2],
                                        payoff1.bCoefficients()[z.size() - 2],
                                        payoff1.aCoefficients()[z.size() - 2],
                                        p[z.size() - 2], z[z.size() - 2],
                                        z[z.size() - 1], 100.0) *
                                    zSpreadDf;
                            if (type == Option::Put)
                                price +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0, payoff1.cCoefficients()[0],
                                        payoff1.bCoefficients()[0],
                                        payoff1.aCoefficients()[0], p[0], z[0],
                                        -100.0, z[0]) *
                                    zSpreadDf;
                        }
                    }
                }

                npv0[k] = price;

                // for probability computation
                if (probabilities_ != None) {
                    for (Size m = 0; m < npvp0.size(); m++) {
                        Real price = 0.0;
                        if (expiry1Time != Null<Real>()) {
                            Real zSpreadDf =
                                oas_.empty()
                                    ? Real(1.0)
                                    : std::exp(-oas_->value() *
                                               (expiry1Time - expiry0Time));
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
                                        z[i], z[i + 1]) *
                                    zSpreadDf;
                            }
                            if (extrapolatePayoff_) {
                                if (flatPayoffExtrapolation_) {
                                    price +=
                                        Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                                  0.0, 0.0, 0.0, 0.0,
                                                  p[z.size() - 2],
                                                  z[z.size() - 2],
                                                  z[z.size() - 1], 100.0) *
                                        zSpreadDf;
                                    price +=
                                        Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                                  0.0, 0.0, 0.0, 0.0, p[0],
                                                  z[0], -100.0, z[0]) *
                                        zSpreadDf;
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
                                                      z[z.size() - 1], 100.0) *
                                            zSpreadDf;
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
                                                      p[0], z[0], -100.0,
                                                      z[0]) *
                                            zSpreadDf;
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
                        Real zSpreadDf =
                            oas_.empty()
                                ? Real(1.0)
                                : std::exp(
                                      -oas_->value() *
                                      (model_->termStructure()
                                           ->dayCounter()
                                           .yearFraction(
                                                expiry0,
                                                arguments_
                                                    .floatingPayDates[l])));
                        Real amount;
                        if (arguments_.floatingIsRedemptionFlow[l])
                            amount = arguments_.floatingCoupons[l];
                        else
                            amount = arguments_.floatingNominal[l] *
                                     arguments_.floatingAccrualTimes[l] *
                                     (arguments_.floatingGearings[l] *
                                          model_->forwardRate(
                                              arguments_.floatingFixingDates[l],
                                              expiry0, z[k],
                                              arguments_.swap->iborIndex()) +
                                      arguments_.floatingSpreads[l]);
                        floatingLegNpv +=
                            amount *
                            model_->zerobond(arguments_.floatingPayDates[l],
                                             expiry0, z[k], discountCurve_) *
                            zSpreadDf;
                    }
                    Real fixedLegNpv = 0.0;
                    for (Size l = j1; l < arguments_.fixedCoupons.size(); l++) {
                        Real zSpreadDf =
                            oas_.empty()
                                ? Real(1.0)
                                : std::exp(
                                      -oas_->value() *
                                      (model_->termStructure()
                                           ->dayCounter()
                                           .yearFraction(
                                                expiry0,
                                                arguments_.fixedPayDates[l])));
                        fixedLegNpv +=
                            arguments_.fixedCoupons[l] *
                            model_->zerobond(arguments_.fixedPayDates[l],
                                             expiry0, z[k], discountCurve_) *
                            zSpreadDf;
                    }
                    Real rebate = 0.0;
                    Real zSpreadDf = 1.0;
                    Date rebateDate = expiry0;
                    if (rebatedExercise != nullptr) {
                        rebate = rebatedExercise->rebate(idx);
                        rebateDate = rebatedExercise->rebatePaymentDate(idx);
                        zSpreadDf =
                            oas_.empty()
                                ? Real(1.0)
                                : std::exp(
                                      -oas_->value() *
                                      (model_->termStructure()
                                           ->dayCounter()
                                           .yearFraction(expiry0, rebateDate)));
                    }
                    Real exerciseValue =
                        ((type == Option::Call ? 1.0 : -1.0) *
                             (floatingLegNpv - fixedLegNpv) +
                         rebate * model_->zerobond(rebateDate, expiry0, z[k],
                                                   discountCurve_) *
                             zSpreadDf) /
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
                                    ? Real(1.0)
                                    : 1.0 / (model_->zerobond(expiry0Time, 0.0,
                                                              0.0,
                                                              discountCurve_) *
                                             model_->numeraire(expiry0, z[k],
                                                               discountCurve_));
                        if (exerciseValue >= npv0[k]) {
                            npvp0[idx - minIdxAlive][k] =
                                probabilities_ == Naive
                                    ? Real(1.0)
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
