/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include <ql/experimental/fxslv/fxsmilesection.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace QuantLib {

    FxSmileSection::FxSmileSection(const Date& exerciseDate,
                                   const Handle<Quote>& spot,
                                   const ext::shared_ptr<FxSmileQuotes>& quotes,
                                   const Handle<YieldTermStructure>& foreignDiscount,
                                   const Handle<YieldTermStructure>& domesticDiscount,
                                   DeltaVolQuote::DeltaType deltaType,
                                   DeltaVolQuote::AtmType atmType,
                                   const DayCounter& dayCounter,
                                   const FxSettleConvention& settlement,
                                   const Date& referenceDate)
    : SmileSection(exerciseDate, dayCounter, referenceDate, ShiftedLognormal, 0.0),
      deltaType_(deltaType), atmType_(atmType), spot_(spot), smileQuotes_(quotes),
      foreignDiscount_(foreignDiscount), domesticDiscount_(domesticDiscount),
      settleConvention_(settlement) {
        QL_REQUIRE(smileQuotes_, "no smile quotes given");
        registerWithMarketData();
    }

    FxSmileSection::FxSmileSection(Time exerciseTime,
                                   const Handle<Quote>& spot,
                                   const ext::shared_ptr<FxSmileQuotes>& quotes,
                                   const Handle<YieldTermStructure>& foreignDiscount,
                                   const Handle<YieldTermStructure>& domesticDiscount,
                                   DeltaVolQuote::DeltaType deltaType,
                                   DeltaVolQuote::AtmType atmType,
                                   const DayCounter& dayCounter)
    : SmileSection(exerciseTime, dayCounter, ShiftedLognormal, 0.0),
      deltaType_(deltaType), atmType_(atmType), spot_(spot), smileQuotes_(quotes),
      foreignDiscount_(foreignDiscount), domesticDiscount_(domesticDiscount) {
        QL_REQUIRE(smileQuotes_, "no smile quotes given");
        registerWithMarketData();
    }

    void FxSmileSection::registerWithMarketData() {
        registerWith(spot_);
        registerWith(foreignDiscount_);
        registerWith(domesticDiscount_);
        registerWith(smileQuotes_);
    }

    namespace {

        void checkCurveValidity(const Handle<YieldTermStructure>& curve,
                                const std::string& name,
                                const Date& spotDate,
                                const Date& deliveryDate) {
            QL_REQUIRE(!curve.empty(), name << " discount curve is empty");
            QL_REQUIRE(curve->referenceDate() <= spotDate,
                       name << " discount curve reference date (" << curve->referenceDate()
                            << ") is after the spot date (" << spotDate << ")");
            QL_REQUIRE(curve->allowsExtrapolation() || deliveryDate <= curve->maxDate(),
                       name << " discount curve ends (" << curve->maxDate()
                            << ") before the delivery date (" << deliveryDate << ")");
        }

    }

    Date FxSmileSection::spotDate() const {
        QL_REQUIRE(settleConvention_, "spot date is only defined for sections built from dates");
        calculate();
        return spotDate_;
    }

    Date FxSmileSection::deliveryDate() const {
        QL_REQUIRE(settleConvention_, "delivery date is only defined for sections built from dates");
        calculate();
        return deliveryDate_;
    }

    FxDeltaConverter FxSmileSection::deltaConverter() const {
        calculate();
        return FxDeltaConverter(deltaType_, spot_->value(), ddom_, dfor_);
    }

    Rate FxSmileSection::peakCallDeltaStrike() const {
        QL_REQUIRE(premiumAdjust(), "the call delta only peaks for premium-adjusted deltas");

        // call delta of the current smile as a function of log-strike
        const FxDeltaConverter conv = deltaConverter();
        const Real sqrtT = std::sqrt(exerciseTime());
        auto callDelta = [&](Real x) {
            const Rate strike = std::exp(x);
            const Volatility vol = volByStrike(strike);
            QL_REQUIRE(std::isfinite(vol) && vol >= 0.0,
                       "smile not defined at strike "
                           << strike << " (vol " << vol
                           << ") while looking for the peak premium-adjusted call delta");
            return conv.delta(Option::Call, strike, vol * sqrtT);
        };

        // bracket the peak, walking from the forward in steps of one
        // reference standard deviation.  The peak is within a step or two of
        // the forward, so the steps do not grow: the walk then never goes
        // more than a step past the peak, where a smile may be undefined.
        Real h = referenceVol() * sqrtT;
        Real b = std::log(fwd_), fb = callDelta(b);
        Real a = b - h, fa = callDelta(a);
        if (fa < fb) {
            // the peak is above the forward: walk up instead
            std::swap(a, b);
            std::swap(fa, fb);
            h = -h;
        }
        Real c = a - h, fc = callDelta(c);
        Size steps = 0;
        while (fc > fa) {
            QL_REQUIRE(++steps < 100, "no peak of the premium-adjusted call delta found");
            b = a;
            fb = fa;
            a = c;
            fa = fc;
            c = a - h;
            fc = callDelta(c);
        }
        // the peak lies between c and b, with a inside and fa the highest

        // golden-section search
        const Real g = 0.5 * (3.0 - std::sqrt(5.0));
        Real lo = std::min(b, c), hi = std::max(b, c);
        Real x1 = lo + g * (hi - lo), x2 = hi - g * (hi - lo);
        Real f1 = callDelta(x1), f2 = callDelta(x2);
        while (hi - lo > 1.0e-10) {
            if (f1 > f2) {
                hi = x2;
                x2 = x1;
                f2 = f1;
                x1 = lo + g * (hi - lo);
                f1 = callDelta(x1);
            } else {
                lo = x1;
                x1 = x2;
                f1 = f2;
                x2 = hi - g * (hi - lo);
                f2 = callDelta(x2);
            }
        }
        return std::exp(0.5 * (lo + hi));
    }

    Real FxSmileSection::maxCallDelta() const {
        const FxDeltaConverter conv = deltaConverter();
        if (!conv.premiumAdjusted())
            return conv.callDeltaLimit();
        const Rate strike = peakCallDeltaStrike();
        return conv.delta(Option::Call, strike, volByStrike(strike) * std::sqrt(exerciseTime()));
    }

    const Array& FxSmileSection::calibrationResiduals() const {
        calculate();
        return calibrationResiduals_;
    }

    Real FxSmileSection::calibrationError() const {
        calculate();
        Real sum = 0.0;
        for (Real r : calibrationResiduals_)
            sum += r * r;
        return std::sqrt(sum / static_cast<Real>(calibrationResiduals_.size()));
    }

    Real FxSmileSection::volResidual(Rate strike, Volatility vol) const {
        return volByStrike(strike) - vol;
    }

    Real FxSmileSection::volDerivative(Rate strike) const {
        QL_REQUIRE(strike > 0.0, "positive strike required: " << strike);
        // central difference; vols are far smoother than prices, so a
        // relative step of 1e-4 balances truncation and rounding errors
        const Real h = 1.0e-4 * strike;
        return (volByStrike(strike + h) - volByStrike(strike - h)) / (2.0 * h);
    }

    Real FxSmileSection::normedCallPrice(Real moneyness) const {
        QL_REQUIRE(moneyness > 0.0, "positive moneyness required: " << moneyness);
        calculate();

        const Rate strike = moneyness * fwd_;
        const Real w = volByStrike(strike) * std::sqrt(exerciseTime());
        return BlackCalculator(Option::Call, strike, fwd_, w).value() / fwd_;
    }

    Probability FxSmileSection::exerciseProbability(Real moneyness) const {
        QL_REQUIRE(moneyness > 0.0, "positive moneyness required: " << moneyness);
        calculate();

        // P(S_T > K) = -dC/dK for the undiscounted call C(K) = Black(F, K, vol(K)):
        //            = N(d2) - F n(d1) sqrt(T) vol'(K)
        const Rate strike = moneyness * fwd_;
        const Time tau = exerciseTime();
        const Real w = volByStrike(strike) * std::sqrt(tau);
        QL_REQUIRE(w > 0.0, "positive standard deviation required at strike " << strike);
        const Real d1 = (std::log(fwd_ / strike) + 0.5 * w * w) / w;
        const Real d2 = d1 - w;

        CumulativeNormalDistribution N;
        NormalDistribution n;
        return N(d2) - fwd_ * n(d1) * std::sqrt(tau) * volDerivative(strike);
    }

    Real FxSmileSection::moneynessFromProbability(Probability p) const {
        QL_REQUIRE(p > 0.0 && p < 1.0, "probability must be in (0, 1): " << p);
        calculate();

        // Solve in log-moneyness, starting from the flat-smile answer
        // N(d2) = p at the ATM vol and widening the bracket as needed.
        const Real stdDev = atmVol_ * std::sqrt(exerciseTime());
        const Real guess = -stdDev * InverseCumulativeNormal()(p) - 0.5 * stdDev * stdDev;
        auto error = [&](Real x) { return exerciseProbability(std::exp(x)) - p; };

        Brent solver;
        solver.setMaxEvaluations(1000);
        const Real k = std::exp(solver.solve(error, 1.0e-12, guess, stdDev));

        // The inverse is only unique where the density is non-negative,
        // i.e. the exercise probability decreases with moneyness.
        const Real h = 1.0e-4 * k;
        QL_ENSURE(exerciseProbability(k + h) <= exerciseProbability(k - h) + 1.0e-12,
                  "negative density at moneyness " << k
                                                   << ": the smile has butterfly arbitrage there");
        return k;
    }

    void FxSmileSection::calculateForward() const {
        if (settleConvention_) {
            // Date mode: rates run from the spot date of the trade date to
            // the delivery date of the expiry. Taking ratios of discount
            // factors by date makes the result independent of the curves'
            // own reference dates and day counters.
            spotDate_ = settleConvention_->valueDate(referenceDate());
            deliveryDate_ = settleConvention_->valueDate(exerciseDate());
            checkCurveValidity(foreignDiscount_, "foreign", spotDate_, deliveryDate_);
            checkCurveValidity(domesticDiscount_, "domestic", spotDate_, deliveryDate_);
            dfor_ = foreignDiscount_->discount(deliveryDate_) / foreignDiscount_->discount(spotDate_);
            ddom_ = domesticDiscount_->discount(deliveryDate_) / domesticDiscount_->discount(spotDate_);
        } else {
            // Time mode: the expiry time is read on the curves' time axis,
            // which is only well defined if both curves share it.
            QL_REQUIRE(!foreignDiscount_.empty() && !domesticDiscount_.empty(), "empty discount curve");
            QL_REQUIRE(foreignDiscount_->referenceDate() == domesticDiscount_->referenceDate(),
                       "time-based FX smile section requires both discount curves to have the "
                       "same reference date: foreign "
                           << foreignDiscount_->referenceDate() << ", domestic "
                           << domesticDiscount_->referenceDate());
            QL_REQUIRE(foreignDiscount_->dayCounter() == domesticDiscount_->dayCounter(),
                       "time-based FX smile section requires both discount curves to have the "
                       "same day counter: foreign "
                           << foreignDiscount_->dayCounter() << ", domestic "
                           << domesticDiscount_->dayCounter());
            dfor_ = foreignDiscount_->discount(exerciseTime());
            ddom_ = domesticDiscount_->discount(exerciseTime());
        }
        fwd_ = spot_->value() * dfor_ / ddom_;
    }

    void FxSmileSection::calculateAtm() const {
        // Called from performCalculations() after the fit.  The ATM strike
        // is the fixed point K = K_atm(vol(K)); search in log-strike from
        // the forward, in steps of one reference standard deviation, widening as
        // needed, so that far-away strikes where a smile may be undefined
        // are only visited if the root is really out there.
        const FxDeltaConverter conv = deltaConverter();
        const Real stdDev = referenceVol() * std::sqrt(exerciseTime());

        auto atmStrikeError = [&](Real logStrike) {
            const Real strike = std::exp(logStrike);
            const Volatility v = volByStrike(strike);
            const Real kAtm = conv.atmStrike(atmType(), v * std::sqrt(exerciseTime()));
            return logStrike - std::log(kAtm);
        };

        Brent solver;
        solver.setMaxEvaluations(1000);
        const Rate k = std::exp(solver.solve(atmStrikeError, 1.0e-12, std::log(fwd_), stdDev));

        atmStrike_ = k;
        atmVol_ = volByStrike(k);
    }

    void FxSmileSection::stripDeltaVolQuotes() const {
        // No ATM vol until the new smile is fitted; the fit is seeded from
        // referenceVol(), so the same quotes always give the same smile.
        atmVol_ = Null<Volatility>();
        atmStrike_ = Null<Real>();
        calibrationResiduals_ = Array();

        // The quotes drive the fit; check they actually fitted the section,
        // otherwise it would silently keep a previous calibration.
        calibrating_ = true;
        fittedToTargets_ = false;
        try {
            smileQuotes_->calibrate(*this);
        } catch (...) {
            calibrating_ = false;
            throw;
        }
        calibrating_ = false;
        QL_ENSURE(fittedToTargets_, "smile quotes did not fit the smile section");

        // the calibrated atm might differ from the quoted one, so take it
        // from the calibrated smile
        calculateAtm();
    }

    void FxSmileSection::performCalculations() const {
        calculateForward();
        stripDeltaVolQuotes();
    }

    void FxSmileSection::fitToTargets(FxSmileTargets targets) const {
        QL_REQUIRE(calibrating_, "smile section can only be fitted while its quotes calibrate it");
        QL_REQUIRE(!targets.empty(), "no calibration targets");
        targets_ = std::move(targets);
        calibrate();

        // measure the fit the same way whatever the model's calibrate() did
        calibrationResiduals_ = Array(targets_.size());
        for (Size i = 0; i < targets_.size(); ++i) {
            calibrationResiduals_[i] = targets_[i]->residual(*this);
            QL_ENSURE(std::isfinite(calibrationResiduals_[i]),
                      "calibrated smile gives a non-finite residual for target " << i);
        }
        fittedToTargets_ = true;
    }

    void FxSmileSection::calibrate() const {
        // Least squares over the model parameters: each evaluation loads
        // the trial parameters, so the targets measure the trial smile
        // through the section's own functions.  Residuals are weighted by
        // vega, so that errors count roughly as price errors.
        Array weights(targets_.size());
        for (Size i = 0; i < targets_.size(); ++i) {
            weights[i] = targets_[i]->weight(*this);
            QL_REQUIRE(weights[i] > 0.0 && std::isfinite(weights[i]),
                       "calibration target " << i << " has an invalid weight: " << weights[i]);
        }

        auto residuals = [&](const Array& params) -> Array {
            setParams(params);
            Array r(targets_.size());
            for (Size i = 0; i < targets_.size(); ++i)
                r[i] = weights[i] * targets_[i]->residual(*this);
            return r;
        };

        SimpleCostFunction<decltype(residuals)> costFunction(residuals);
        NoConstraint constraint;
        Problem problem(costFunction, constraint, initialParams());
        LevenbergMarquardt lm;
        EndCriteria endCriteria(1000, 100, 1.0e-12, 1.0e-12, 1.0e-12);
        const EndCriteria::Type result = lm.minimize(problem, endCriteria);
        // MINPACK's "cannot reduce further" (FunctionEpsilonTooSmall) is a
        // converged fit too, typically an exact one
        QL_ENSURE(EndCriteria::succeeded(result) || result == EndCriteria::FunctionEpsilonTooSmall,
                  "smile calibration did not converge: " << result);

        setParams(problem.currentValue());
    }

}
