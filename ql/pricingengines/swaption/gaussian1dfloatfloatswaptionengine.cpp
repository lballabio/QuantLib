/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

#include <ql/pricingengines/swaption/gaussian1dfloatfloatswaptionengine.hpp>
#include <ql/experimental/coupons/swapspreadindex.hpp> // internal
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    void Gaussian1dFloatFloatSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "Gaussian1dFloatFloatSwaptionEngine");

        Date settlement = model_->termStructure()->referenceDate();

        if (arguments_.exercise->dates().back() <=
            settlement) { // swaption is expired,
                          // possibly generated swap
                          // is not
                          // valued
            results_.value = 0.0;
            return;
        }

        rebatedExercise_ =
            ext::dynamic_pointer_cast<RebatedExercise>(arguments_.exercise);

        std::pair<Real, Real> result =
            npvs(settlement, 0.0, includeTodaysExercise_, true);

        results_.value = result.first;
        results_.additionalResults["underlyingValue"] = result.second;
    }

    Real
    Gaussian1dFloatFloatSwaptionEngine::underlyingNpv(const Date &expiry,
                                                      const Real y) const {
        return npvs(expiry, y, true).second;
    }

    Swap::Type Gaussian1dFloatFloatSwaptionEngine::underlyingType() const {
        return arguments_.swap->type();
    }

    // NOLINTNEXTLINE(readability-const-return-type)
    const Date Gaussian1dFloatFloatSwaptionEngine::underlyingLastDate() const {
        Date l1 = arguments_.leg1PayDates.back();
        Date l2 = arguments_.leg2PayDates.back();
        return l2 >= l1 ? l2 : l1;
    }

    // NOLINTNEXTLINE(readability-const-return-type)
    const Array Gaussian1dFloatFloatSwaptionEngine::initialGuess(const Date &expiry) const {

        Size idx1 =
            std::upper_bound(arguments_.leg1ResetDates.begin(),
                             arguments_.leg1ResetDates.end(), expiry - 1) -
            arguments_.leg1ResetDates.begin();

        // very simple initial guess
        // check guess for nominal and weighted maturity !

        Real nominalSum1 = 0.0;
        for (Size i = idx1; i < arguments_.leg1ResetDates.size(); i++) {
            nominalSum1 += arguments_.nominal1[i];
        }
        Real nominalAvg1 = nominalSum1 /
            (arguments_.leg1ResetDates.size() - idx1);
        Real weightedMaturity1 = 0.0;
        for (Size i = idx1; i < arguments_.leg1ResetDates.size(); i++) {
            weightedMaturity1 +=
                arguments_.leg1AccrualTimes[i] * arguments_.nominal1[i];
        }
        weightedMaturity1 /= nominalAvg1;

        return {
            nominalAvg1,
            weightedMaturity1,
            0.03 // ???
        };
    }

    // calculate npv and underlying npv as of expiry date
    std::pair<Real, Real>
    Gaussian1dFloatFloatSwaptionEngine::npvs(const Date& expiry,
                                             const Real y,
                                             const bool includeExerciseOnExpiry,
                                             const bool considerProbabilities) const {

        // pricing

        // event dates are coupon fixing dates and exercise dates
        // we explicitly estimate cms and also libor coupons (although
        // the latter could be calculated analytically) to make the code
        // simpler

        std::vector<Date> events;
        events.insert(events.end(), arguments_.exercise->dates().begin(),
                      arguments_.exercise->dates().end());
        events.insert(events.end(), arguments_.leg1FixingDates.begin(),
                      arguments_.leg1FixingDates.end());
        events.insert(events.end(), arguments_.leg2FixingDates.begin(),
                      arguments_.leg2FixingDates.end());
        std::sort(events.begin(), events.end());

        auto it = std::unique(events.begin(), events.end());
        events.resize(std::distance(events.begin(), it));

        // only events on or after expiry are of interest by definition of the
        // deal part that is exericsed into.

        auto filit = std::upper_bound(events.begin(), events.end(),
                                      expiry - (includeExerciseOnExpiry ? 1 : 0));
        events.erase(events.begin(), filit);

        int idx = events.size() - 1;

        FloatFloatSwap swap = *arguments_.swap;
        Option::Type type =
            arguments_.type == Swap::Payer ? Option::Call : Option::Put;

        Array npv0(2 * integrationPoints_ + 1, 0.0),
            npv1(2 * integrationPoints_ + 1, 0.0); // arrays for npvs of the
                                                   // option
        Array npv0a(2 * integrationPoints_ + 1, 0.0),
            npv1a(2 * integrationPoints_ + 1, 0.0); // arrays for npvs of the
                                                    // underlying
        Array z = model_->yGrid(stddevs_, integrationPoints_);
        Array p(z.size(), 0.0), pa(z.size(), 0.0);

        // for probability computation
        std::vector<Array> npvp0, npvp1;
        // how many active exercise dates are there ?
        Size noEx =  arguments_.exercise->dates().size() -
            (std::upper_bound(arguments_.exercise->dates().begin(),
                         arguments_.exercise->dates().end(),
                         expiry - (includeExerciseOnExpiry ? 1 : 0)) -
             arguments_.exercise->dates().begin());
        Size exIdx = noEx; // current exercise index
        if (considerProbabilities && probabilities_ != None) {
            for (Size i = 0; i < noEx+1 ; ++i) {
                Array npvTmp0(2 * integrationPoints_ + 1, 0.0);
                Array npvTmp1(2 * integrationPoints_ + 1, 0.0);
                npvp0.push_back(npvTmp0);
                npvp1.push_back(npvTmp1);
            }
        }
        // end probability computation

        Date event1 = Date(), event0;
        Time event1Time = Null<Real>(), event0Time;

        ext::shared_ptr<IborIndex> ibor1 =
            ext::dynamic_pointer_cast<IborIndex>(arguments_.index1);
        ext::shared_ptr<SwapIndex> cms1 =
            ext::dynamic_pointer_cast<SwapIndex>(arguments_.index1);
        ext::shared_ptr<SwapSpreadIndex> cmsspread1 =
            ext::dynamic_pointer_cast<SwapSpreadIndex>(arguments_.index1);
        ext::shared_ptr<IborIndex> ibor2 =
            ext::dynamic_pointer_cast<IborIndex>(arguments_.index2);
        ext::shared_ptr<SwapIndex> cms2 =
            ext::dynamic_pointer_cast<SwapIndex>(arguments_.index2);
        ext::shared_ptr<SwapSpreadIndex> cmsspread2 =
            ext::dynamic_pointer_cast<SwapSpreadIndex>(arguments_.index2);

        QL_REQUIRE(ibor1 != nullptr || cms1 != nullptr || cmsspread1 != nullptr,
                   "index1 must be ibor or swap or swap spread index");
        QL_REQUIRE(ibor2 != nullptr || cms2 != nullptr || cmsspread2 != nullptr,
                   "index2 must be ibor or swap or swap spread index");

        do {

            // we are at event0 date, which can be a structured coupon fixing
            // date or an exercise date or both.

            bool isEventDate = true;
            if (idx == -1) {
                event0 = expiry;
                isEventDate = false;
            } else {
                event0 = events[idx];
                if (event0 == expiry)
                    idx = -1; // avoid double roll back if expiry equal to
                              // earliest event date
            }

            bool isExercise =
                std::find(arguments_.exercise->dates().begin(), arguments_.exercise->dates().end(),
                          event0) != arguments_.exercise->dates().end();

            bool isLeg1Fixing =
                std::find(arguments_.leg1FixingDates.begin(), arguments_.leg1FixingDates.end(),
                          event0) != arguments_.leg1FixingDates.end();

            bool isLeg2Fixing =
                std::find(arguments_.leg2FixingDates.begin(), arguments_.leg2FixingDates.end(),
                          event0) != arguments_.leg2FixingDates.end();

            event0Time = std::max(
                model_->termStructure()->timeFromReference(event0), 0.0);

            // todo add openmp support later on (as in gaussian1dswaptionengine)

            for (Size k = 0; k < (event0 > expiry ? npv0.size() : 1); k++) {

                // roll back

                Real price = 0.0, pricea = 0.0;
                if (event1Time != Null<Real>()) {
                    Real zSpreadDf = oas_.empty()
                                         ? Real(1.0)
                                         : std::exp(-oas_->value() *
                                                    (event1Time - event0Time));
                    Array yg =
                        model_->yGrid(stddevs_, integrationPoints_, event1Time,
                                      event0Time, event0 > expiry ? z[k] : y);
                    CubicInterpolation payoff0(
                        z.begin(), z.end(), npv1.begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                    CubicInterpolation payoff0a(
                        z.begin(), z.end(), npv1a.begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                    for (Size i = 0; i < yg.size(); i++) {
                        p[i] = payoff0(yg[i], true);
                        pa[i] = payoff0a(yg[i], true);
                    }
                    CubicInterpolation payoff1(
                        z.begin(), z.end(), p.begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                    CubicInterpolation payoff1a(
                        z.begin(), z.end(), pa.begin(),
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
                        pricea += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                      0.0, payoff1a.cCoefficients()[i],
                                      payoff1a.bCoefficients()[i],
                                      payoff1a.aCoefficients()[i], pa[i], z[i],
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
                            pricea +=
                                Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                    0.0, 0.0, 0.0, 0.0, pa[z.size() - 2],
                                    z[z.size() - 2], z[z.size() - 1], 100.0) *
                                zSpreadDf;
                            pricea += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                          0.0, 0.0, 0.0, 0.0, pa[0], z[0],
                                          -100.0, z[0]) *
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
                            if (type == Option::Call)
                                pricea +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0,
                                        payoff1a.cCoefficients()[z.size() - 2],
                                        payoff1a.bCoefficients()[z.size() - 2],
                                        payoff1a.aCoefficients()[z.size() - 2],
                                        pa[z.size() - 2], z[z.size() - 2],
                                        z[z.size() - 1], 100.0) *
                                    zSpreadDf;
                            if (type == Option::Put)
                                pricea +=
                                    Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                        0.0, payoff1a.cCoefficients()[0],
                                        payoff1a.bCoefficients()[0],
                                        payoff1a.aCoefficients()[0], pa[0],
                                        z[0], -100.0, z[0]) *
                                    zSpreadDf;
                        }
                    }
                }

                npv0[k] = price;
                npv0a[k] = pricea;

                // for probability computation
                if (considerProbabilities && probabilities_ != None) {
                    for (Size m = 0; m < npvp0.size(); m++) {
                        Real price = 0.0;
                        if (event1Time != Null<Real>()) {
                            Real zSpreadDf =
                                oas_.empty()
                                    ? Real(1.0)
                                    : std::exp(-oas_->value() *
                                               (event1Time - event0Time));
                            Array yg = model_->yGrid(
                                stddevs_, integrationPoints_, event1Time,
                                event0Time, event0 > expiry ? z[k] : 0.0);
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

                // event date calculations

                if (isEventDate) {

                    Real zk = event0 > expiry ? z[k] : y;

                    if (isLeg1Fixing) { // if event is a fixing date and
                                        // exercise date,
                        // the coupon is part of the exercise into right (by
                        // definition)
                        Size j = std::find(arguments_.leg1FixingDates.begin(),
                                           arguments_.leg1FixingDates.end(),
                                           event0) -
                                 arguments_.leg1FixingDates.begin();
                        Real zSpreadDf =
                            oas_.empty()
                                ? Real(1.0)
                                : std::exp(
                                      -oas_->value() *
                                      (model_->termStructure()
                                           ->dayCounter()
                                           .yearFraction(
                                                event0,
                                                arguments_.leg1PayDates[j])));
                        bool done = false;
                        do {
                            Real amount;
                            if (arguments_.leg1IsRedemptionFlow[j]) {
                                amount = arguments_.leg1Coupons[j];
                            } else {
                                Real estFixing = 0.0;
                                if (ibor1 != nullptr) {
                                    estFixing = model_->forwardRate(
                                        arguments_.leg1FixingDates[j], event0,
                                        zk, ibor1);
                                }
                                if (cms1 != nullptr) {
                                    estFixing = model_->swapRate(
                                        arguments_.leg1FixingDates[j],
                                        cms1->tenor(), event0, zk, cms1);
                                }
                                if (cmsspread1 != nullptr)
                                    estFixing =
                                        cmsspread1->gearing1() *
                                            model_->swapRate(
                                                arguments_.leg1FixingDates[j],
                                                cmsspread1->swapIndex1()
                                                    ->tenor(),
                                                event0, zk,
                                                cmsspread1->swapIndex1()) +
                                        cmsspread1->gearing2() *
                                            model_->swapRate(
                                                arguments_.leg1FixingDates[j],
                                                cmsspread1->swapIndex2()
                                                    ->tenor(),
                                                event0, zk,
                                                cmsspread1->swapIndex2());
                                Real rate =
                                    arguments_.leg1Spreads[j] +
                                    arguments_.leg1Gearings[j] * estFixing;
                                if (arguments_.leg1CappedRates[j] !=
                                    Null<Real>())
                                    rate = std::min(
                                        arguments_.leg1CappedRates[j], rate);
                                if (arguments_.leg1FlooredRates[j] !=
                                    Null<Real>())
                                    rate = std::max(
                                        arguments_.leg1FlooredRates[j], rate);
                                amount = rate * arguments_.nominal1[j] *
                                         arguments_.leg1AccrualTimes[j];
                            }

                            npv0a[k] -=
                                amount *
                                model_->zerobond(arguments_.leg1PayDates[j],
                                                 event0, zk, discountCurve_) /
                                model_->numeraire(event0Time, zk,
                                                  discountCurve_) *
                                zSpreadDf;

                            if (j < arguments_.leg1FixingDates.size() - 1) {
                                j++;
                                done =
                                    (event0 != arguments_.leg1FixingDates[j]);
                            } else
                                done = true;

                        } while (!done);
                    }

                    if (isLeg2Fixing) { // if event is a fixing date and
                                        // exercise date,
                        // the coupon is part of the exercise into right (by
                        // definition)
                        Size j = std::find(arguments_.leg2FixingDates.begin(),
                                           arguments_.leg2FixingDates.end(),
                                           event0) -
                                 arguments_.leg2FixingDates.begin();
                        Real zSpreadDf =
                            oas_.empty()
                                ? Real(1.0)
                                : std::exp(
                                      -oas_->value() *
                                      (model_->termStructure()
                                           ->dayCounter()
                                           .yearFraction(
                                                event0,
                                                arguments_.leg2PayDates[j])));
                        bool done;
                        do {
                            Real amount;
                            if (arguments_.leg2IsRedemptionFlow[j]) {
                                amount = arguments_.leg2Coupons[j];
                            } else {
                                Real estFixing = 0.0;
                                if (ibor2 != nullptr)
                                    estFixing = model_->forwardRate(arguments_.leg2FixingDates[j],event0,zk,ibor2);
                                if (cms2 != nullptr)
                                    estFixing = model_->swapRate(arguments_.leg2FixingDates[j],cms2->tenor(),event0,zk,cms2);
                                if (cmsspread2 != nullptr)
                                    estFixing =
                                        cmsspread2->gearing1() *
                                            model_->swapRate(
                                                arguments_.leg2FixingDates[j],
                                                cmsspread2->swapIndex1()
                                                    ->tenor(),
                                                event0, zk,
                                                cmsspread2->swapIndex1()) +
                                        cmsspread2->gearing2() *
                                            model_->swapRate(
                                                arguments_.leg2FixingDates[j],
                                                cmsspread2->swapIndex2()
                                                    ->tenor(),
                                                event0, zk,
                                                cmsspread2->swapIndex2());
                                Real rate =
                                    arguments_.leg2Spreads[j] +
                                    arguments_.leg2Gearings[j] * estFixing;
                                if (arguments_.leg2CappedRates[j] !=
                                    Null<Real>())
                                    rate = std::min(
                                        arguments_.leg2CappedRates[j], rate);
                                if (arguments_.leg2FlooredRates[j] !=
                                    Null<Real>())
                                    rate = std::max(
                                        arguments_.leg2FlooredRates[j], rate);
                                amount = rate * arguments_.nominal2[j] *
                                         arguments_.leg2AccrualTimes[j];
                            }

                            npv0a[k] +=
                                amount *
                                model_->zerobond(arguments_.leg2PayDates[j],
                                                 event0, zk, discountCurve_) /
                                model_->numeraire(event0Time, zk,
                                                  discountCurve_) *
                                zSpreadDf;
                            if (j < arguments_.leg2FixingDates.size() - 1) {
                                j++;
                                done =
                                    (event0 != arguments_.leg2FixingDates[j]);
                            } else
                                done = true;

                        } while (!done);
                    }

                    if (isExercise) {
                        Size j = std::find(arguments_.exercise->dates().begin(),
                                           arguments_.exercise->dates().end(),
                                           event0) -
                                 arguments_.exercise->dates().begin();
                        Real rebate = 0.0;
                        Real zSpreadDf = 1.0;
                        Date rebateDate = event0;
                        if (rebatedExercise_ != nullptr) {
                            rebate = rebatedExercise_->rebate(j);
                            rebateDate = rebatedExercise_->rebatePaymentDate(j);
                            zSpreadDf =
                                oas_.empty()
                                    ? Real(1.0)
                                    : std::exp(-oas_->value() *
                                               (model_->termStructure()
                                                    ->dayCounter()
                                                    .yearFraction(event0,
                                                                  rebateDate)));
                        }
                        Real exerciseValue =
                            (type == Option::Call ? 1.0 : -1.0) * npv0a[k] +
                            rebate * model_->zerobond(rebateDate, event0) *
                                zSpreadDf / model_->numeraire(event0Time, zk,
                                                              discountCurve_);

                        if (considerProbabilities && probabilities_ != None) {
                            if (exIdx == noEx) {
                                // if true we are at the latest date,
                                // so we init
                                // the no call probability
                                npvp0.back()[k] =
                                    probabilities_ == Naive
                                        ? Real(1.0)
                                        : 1.0 / (model_->zerobond(
                                                     event0Time, 0.0, 0.0,
                                                     discountCurve_) *
                                                 model_->numeraire(
                                                     event0, z[k],
                                                     discountCurve_));
                            }
                            if (exerciseValue >= npv0[k]) {
                                npvp0[exIdx-1][k] =
                                    probabilities_ == Naive
                                        ? Real(1.0)
                                        : 1.0 / (model_->zerobond(
                                                     event0Time, 0.0, 0.0,
                                                     discountCurve_) *
                                                 model_->numeraire(
                                                     event0Time, z[k],
                                                     discountCurve_));
                                for (Size ii = exIdx; ii < noEx+1; ++ii)
                                    npvp0[ii][k] = 0.0;
                            }
                        }
                        // end probability computation

                        npv0[k] = std::max(npv0[k], exerciseValue);
                    }
                }
            }

            if(isExercise)
                --exIdx;

            npv1.swap(npv0);
            npv1a.swap(npv0a);

            // for probability computation
            if(considerProbabilities && probabilities_ != None) {
                for(Size i=0;i<npvp0.size();++i) {
                    npvp1[i].swap(npvp0[i]);
                }
            }
            // end probability computation

            event1 = event0;
            event1Time = event0Time;

        } while (--idx >= -1);

        std::pair<Real, Real> res(
            npv1[0] * model_->numeraire(event1Time, y, discountCurve_),
            npv1a[0] * model_->numeraire(event1Time, y, discountCurve_) *
                (type == Option::Call ? 1.0 : -1.0));

        // for probability computation
        if (considerProbabilities && probabilities_ != None) {
            std::vector<Real> prob(noEx+1);
            for (Size i = 0; i < noEx+1; i++) {
                prob[i] = npvp1[i][0] *
                          (probabilities_ == Naive
                               ? 1.0
                               : model_->numeraire(0.0, 0.0, discountCurve_));
            }
            results_.additionalResults["probabilities"] = prob;
        }
        // end probability computation

        return res;
    }
}
