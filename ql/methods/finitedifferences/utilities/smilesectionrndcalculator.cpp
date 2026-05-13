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
#include <ql/methods/finitedifferences/utilities/smilesectionrndcalculator.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace QuantLib {

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

        const Real forward = smile_->atmLevel();
        QL_REQUIRE(forward != Null<Real>(),
                   "SmileSectionRNDCalculator: smile->atmLevel() returned "
                   "Null<Real>(); wrap with AtmSmileSection to supply one");

        const Time T = smile_->exerciseTime();
        const Real sigmaAtm = smile_->volatility(forward);
        const Real logStd = sigmaAtm * std::sqrt(T);
        const Real kMin = std::max(forward * std::exp(-nStd_ * logStd), QL_EPSILON);
        const Real kMax = forward * std::exp( nStd_ * logStd);

        // Build a strictly-increasing (cdf, strike) grid via 
        // Breeden-Litzenberger:  CDF(K) = 1 - C'(K).
        // The undiscounted digital call (discount=1) gives the risk-neutral
        // probability P(S > K). BL output may be slightly non-monotone
        // near the wings, so we take a running max and drop
        // near-duplicates to give the spline a strict abscissa.
        strikes_.clear();
        cdf_.clear();
        strikes_.reserve(nStrikes_);
        cdf_.reserve(nStrikes_);
        constexpr Real dedupTol = 1e-12;
        Real lastCdf = -1.0;
        for (Size i = 0; i < nStrikes_; ++i) {
            const Real K = kMin + (kMax - kMin) * i / (nStrikes_ - 1);
            const Real c = std::clamp(
                1.0 - smile_->digitalOptionPrice(K, Option::Call, 1.0),
                0.0, 1.0);
            const Real cMono = std::max(c, lastCdf);
            if (cMono - lastCdf > dedupTol) {
                strikes_.push_back(K);
                cdf_.push_back(cMono);
                lastCdf = cMono;
            }
        }
        QL_REQUIRE(cdf_.size() >= 4,
                   "SmileSectionRNDCalculator: too few unique CDF points ("
                   << cdf_.size() << ") after deduplication");

        quantileFn_ = ext::make_shared<MonotonicCubicNaturalSpline>(
            cdf_.begin(), cdf_.end(), strikes_.begin());

        initialized_ = true;
    }

    Real SmileSectionRNDCalculator::pdf(Real x, Time t) const {
        checkTime(t);
        const Real S = std::exp(x);
        return S * smile_->density(S, 1.0);
    }

    Real SmileSectionRNDCalculator::cdf(Real x, Time t) const {
        checkTime(t);
        const Real S = std::exp(x);
        return 1.0 - smile_->digitalOptionPrice(S, Option::Call, 1.0);
    }

    Real SmileSectionRNDCalculator::invcdf(Real p, Time t) const {
        checkTime(t);
        initialize();
        QL_REQUIRE(p > 0.0 && p < 1.0,
                   "p must be in (0, 1), got " << p);
        const Real pClamped = std::min(std::max(p, cdf_.front()), cdf_.back());
        const Real K = (*quantileFn_)(pClamped);
        return std::log(K);
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
