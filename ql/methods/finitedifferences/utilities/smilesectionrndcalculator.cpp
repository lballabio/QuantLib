/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yassine Idyiahia

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

#include <ql/errors.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/utilities/smilesectionrndcalculator.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace {

        /*! P(S > K) for a call, P(S <= K) for a put. Each tail is taken from
            its own option type rather than as 1 - p, so the small number the
            wings need is never the difference of two numbers close to one.

            The gap scales with the strike, since the grid is walked outward
            over decades and a width suiting the forward is far too coarse by
            the wings. It stays well above the underflow floor that
            digitalOptionPrice applies for itself.
        */
        Real tailProbability(const SmileSection& smile, Real strike, Option::Type type) {
            const Real relativeGap = 2.0e-6;
            const Real gap = std::max(std::abs(strike), QL_EPSILON) * relativeGap;
            return std::clamp(smile.digitalOptionPrice(strike, type, 1.0, gap),
                              Real(0.0), Real(1.0));
        }

        //! z = Phi^{-1}(P(S <= K)) at log-moneyness x, or Null<Real>() if unusable.
        Real normalQuantileAt(const SmileSection& smile, Real forward, Real x) {
            const InverseCumulativeNormal invPhi;
            const Real strike = forward * std::exp(x);
            if (!std::isfinite(strike) || strike <= 0.0)
                return Null<Real>();

            const Real p = (x <= 0.0) ? tailProbability(smile, strike, Option::Put)
                                      : tailProbability(smile, strike, Option::Call);
            if (p <= 0.0 || p >= 1.0)
                return Null<Real>();

            const Real z = (x <= 0.0) ? invPhi(p) : -invPhi(p);
            return std::isfinite(z) ? z : Null<Real>();
        }

    }

    SmileSectionRNDCalculator::SmileSectionRNDCalculator(
        ext::shared_ptr<SmileSection> smile,
        Size nStrikes,
        Real nStd)
    : smile_(std::move(smile)), nStrikes_(nStrikes), nStd_(nStd) {
        QL_REQUIRE(smile_, "null SmileSection");
        QL_REQUIRE(nStrikes_ >= 4,
                   "at least 4 strikes required, got " << nStrikes_);
        QL_REQUIRE(nStd_ > 0.0,
                   "nStd must be positive, got " << nStd_);
    }

    void SmileSectionRNDCalculator::checkTime(Time t) const {
        const Time tRef = smile_->exerciseTime();
        QL_REQUIRE(close_enough(t, tRef),
                   "SmileSectionRNDCalculator: requested t=" << t
                   << " does not match smile exercise time " << tRef);
    }

    void SmileSectionRNDCalculator::initialize() const {
        if (initialized_)
            return;

        forward_ = smile_->atmLevel();
        QL_REQUIRE(forward_ != Null<Real>(),
                   "SmileSectionRNDCalculator: smile->atmLevel() returned "
                   "Null<Real>(); wrap with AtmSmileSection to supply one");

        const Time T = smile_->exerciseTime();
        const Real logStd = smile_->volatility(forward_) * std::sqrt(T);
        QL_REQUIRE(logStd > 0.0,
                   "SmileSectionRNDCalculator: non-positive at-the-money "
                   "volatility " << logStd);

        // Fat wings reach a given quantile much further out in strike than a
        // lognormal does, so the extent has to be found rather than assumed.
        const Real zAtm = normalQuantileAt(*smile_, forward_, 0.0);
        QL_REQUIRE(zAtm != Null<Real>(),
                   "SmileSectionRNDCalculator: could not determine the "
                   "quantile at the forward");

        // Doubled while out of reach, since a step tuned to a low ATM vol
        // cannot cross fat wings.
        const Real firstStep = 0.5 * logStd;
        // Ample for doubling out and backing off again.
        constexpr Size maxSteps = 200;
        // Finer than the tabulated map can resolve.
        const Real xTol = 1e-12;

        const auto edge = [&](Real target) {
            Real x = 0.0;
            Real step = firstStep;
            const Real direction = target > zAtm ? 1.0 : -1.0;
            for (Size i = 0; i < maxSteps; ++i) {
                const Real trial = x + direction * step;
                const Real trialZ = normalQuantileAt(*smile_, forward_, trial);
                if (trialZ == Null<Real>()) {
                    // Overshot past where the smile still prices, so back off.
                    step *= 0.5;
                    if (step < xTol)
                        break;
                    continue;
                }

                const bool bracketed =
                    direction > 0.0 ? trialZ >= target : trialZ <= target;
                if (bracketed) {
                    // z increases with x, so this brackets either way.
                    Real xLo = std::min(x, trial);
                    Real xHi = std::max(x, trial);
                    while (xHi - xLo > xTol) {
                        const Real xMid = 0.5 * (xLo + xHi);
                        if (xMid <= xLo || xMid >= xHi)
                            break; // down to one representable step
                        const Real zMid =
                            normalQuantileAt(*smile_, forward_, xMid);
                        QL_REQUIRE(zMid != Null<Real>(),
                                   "SmileSectionRNDCalculator: could not "
                                   "refine a tail quantile");
                        if (zMid < target)
                            xLo = xMid;
                        else
                            xHi = xMid;
                    }
                    return 0.5 * (xLo + xHi);
                }

                x = trial;
                step *= 2.0;
            }
            return x;
        };

        const Real xMax = edge(nStd_);
        const Real xMin = edge(-nStd_);
        QL_REQUIRE(xMax > xMin,
                   "SmileSectionRNDCalculator: could not bracket the quantile range; "
                   "the smile returns no usable tail probability");

        const auto linspace = [](Real lo, Real hi, Size n) {
            std::vector<Real> v(n);
            for (Size i = 0; i < n; ++i)
                v[i] = lo + (hi - lo) * i / (n - 1);
            return v;
        };

        // Sample the map, keeping it strictly increasing so the spline gets a
        // valid abscissa.
        const auto sample = [&](const std::vector<Real>& xs,
                                std::vector<Real>& zOut,
                                std::vector<Real>& xOut) {
            constexpr double dedupTol = 1e-12;
            zOut.clear();
            xOut.clear();
            zOut.reserve(xs.size());
            xOut.reserve(xs.size());
            Real lastZ = -QL_MAX_REAL;
            for (Real x : xs) {
                const Real z = normalQuantileAt(*smile_, forward_, x);
                if (z == Null<Real>())
                    continue;
                if (z > lastZ + dedupTol) {
                    zOut.push_back(z);
                    xOut.push_back(x);
                    lastZ = z;
                }
            }
        };

        // Whatever consumes this samples in z, not in strike, so a grid uniform
        // in log-moneyness puts its nodes in the wrong place once the smile has
        // wings. Locate the map coarsely, then re-place the nodes uniformly in z.
        // The locating pass only resolves the shape, not the node placement.
        constexpr Size maxCoarseNodes = 100;
        const Size nCoarse = std::min(maxCoarseNodes, nStrikes_);
        std::vector<Real> zCoarse, xCoarse;
        sample(linspace(xMin, xMax, nCoarse), zCoarse, xCoarse);
        QL_REQUIRE(zCoarse.size() >= 4,
                   "SmileSectionRNDCalculator: too few usable quantile points ("
                   << zCoarse.size() << ") in the locating pass");

        const MonotonicCubicNaturalSpline coarse(
            zCoarse.begin(), zCoarse.end(), xCoarse.begin());
        std::vector<Real> targets =
            linspace(zCoarse.front(), zCoarse.back(), nStrikes_);
        for (Real& t : targets)
            t = coarse(t);

        sample(targets, z_, logMoneyness_);
        QL_REQUIRE(z_.size() >= 4,
                   "SmileSectionRNDCalculator: too few usable quantile points ("
                   << z_.size() << ") after deduplication");

        quantileFn_ = ext::make_shared<MonotonicCubicNaturalSpline>(
            z_.begin(), z_.end(), logMoneyness_.begin());
        zMin_ = z_.front();
        zMax_ = z_.back();

        initialized_ = true;
    }

    Real SmileSectionRNDCalculator::pdf(Real x, Time t) const {
        checkTime(t);
        const Real S = std::exp(x);
        // density() floors its own gap relative to the strike, which is what
        // this needs walking out over decades, so the default is already right.
        return S * smile_->density(S, 1.0);
    }

    Real SmileSectionRNDCalculator::cdf(Real x, Time t) const {
        checkTime(t);
        // Take each tail from its own option type, as the quantile grid does.
        // atmLevel() may be absent, and cdf() does not otherwise need it.
        const Real K = std::exp(x);
        const Real atm = smile_->atmLevel();
        if (atm != Null<Real>() && K <= atm)
            return tailProbability(*smile_, K, Option::Put);
        return 1.0 - tailProbability(*smile_, K, Option::Call);
    }

    Real SmileSectionRNDCalculator::invcdf(Real p, Time t) const {
        checkTime(t);
        initialize();
        QL_REQUIRE(p > 0.0 && p < 1.0,
                   "p must be in (0, 1), got " << p);
        const InverseCumulativeNormal invPhi;
        const Real z = std::clamp(invPhi(p), zMin_, zMax_);
        return std::log(forward_) + (*quantileFn_)(z);
    }

    Real SmileSectionRNDCalculator::pdf(Real x) const {
        return pdf(x, smile_->exerciseTime());
    }

    Real SmileSectionRNDCalculator::cdf(Real x) const {
        return cdf(x, smile_->exerciseTime());
    }

    Real SmileSectionRNDCalculator::invcdf(Real p) const {
        return invcdf(p, smile_->exerciseTime());
    }

}
