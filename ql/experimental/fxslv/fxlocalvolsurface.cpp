/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
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

#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace QuantLib {

    FxLocalVolSurface::FxLocalVolSurface(const Handle<BlackVolTermStructure>& blackTS,
                                         Handle<YieldTermStructure> riskFreeTS,
                                         Handle<YieldTermStructure> dividendTS,
                                         Handle<Quote> underlying,
                                         Real tolerance,
                                         Volatility minLocalVol,
                                         Volatility fallbackVol)
    : LocalVolTermStructure(blackTS->businessDayConvention(), blackTS->dayCounter()),
      blackTS_(blackTS), riskFreeTS_(std::move(riskFreeTS)), dividendTS_(std::move(dividendTS)),
      underlying_(std::move(underlying)), tolerance_(tolerance), minLocalVol_(minLocalVol),
      fallbackVol_(fallbackVol) {
        QL_REQUIRE(tolerance_ >= 0.0, "negative tolerance: " << tolerance_);
        QL_REQUIRE(minLocalVol_ >= 0.0, "negative local vol floor: " << minLocalVol_);
        QL_REQUIRE(fallbackVol_ == Null<Volatility>() || fallbackVol_ >= 0.0,
                   "negative fallback volatility: " << fallbackVol_);
        registerWith(blackTS_);
        registerWith(riskFreeTS_);
        registerWith(dividendTS_);
        registerWith(underlying_);
    }

    Real FxLocalVolSurface::laterVariance(Real earlier, Real later, Real strike, Time t1,
                                          Time t2) const {
        // a change within the tolerance, either way, is rounding
        if (std::fabs(later - earlier) <= tolerance_ * std::max(earlier, later))
            return earlier;
        QL_REQUIRE(later > earlier, "calendar arbitrage: total variance at strike "
                                        << strike << " decreases from " << earlier << " at time "
                                        << t1 << " to " << later << " at time " << t2);
        return later;
    }

    Volatility FxLocalVolSurface::localVolImpl(Time t, Real underlyingLevel) const {
        if (fallbackVol_ == Null<Volatility>())
            return dupireVol(t, underlyingLevel);
        try {
            return dupireVol(t, underlyingLevel);
        } catch (Error&) {
            ++fallbacks_;
            return fallbackVol_;
        }
    }

    Volatility FxLocalVolSurface::dupireVol(Time t, Real underlyingLevel) const {
        const DiscountFactor dr = riskFreeTS_->discount(t, true);
        const DiscountFactor dq = dividendTS_->discount(t, true);
        const Real forwardValue = underlying_->value() * dq / dr;

        // strike derivatives at the given time
        const Real strike = underlyingLevel;
        const Real y = std::log(strike / forwardValue);
        const Real dy = (std::fabs(y) > 0.001) ? Real(y * 0.0001) : 0.000001;
        const Real strikep = strike * std::exp(dy), strikem = strike / std::exp(dy);
        const Real w = blackTS_->blackVariance(t, strike, true);
        const Real wp = blackTS_->blackVariance(t, strikep, true);
        const Real wm = blackTS_->blackVariance(t, strikem, true);
        const Real dwdy = (wp - wm) / (2.0 * dy);
        const Real d2wdy2 = (wp - 2.0 * w + wm) / (dy * dy);

        // time derivative at the same moneyness, the strike moving with the forward
        Real dwdt;
        if (t == 0.0) {
            const Time dt = 0.0001;
            const Real strikept = strike * dr * dividendTS_->discount(t + dt, true) /
                                  (riskFreeTS_->discount(t + dt, true) * dq);
            const Real wpt =
                laterVariance(w, blackTS_->blackVariance(t + dt, strikept, true), strike, t, t + dt);
            dwdt = (wpt - w) / dt;
        } else {
            const Time dt = std::min<Time>(0.0001, t / 2.0);
            const Real strikept = strike * dr * dividendTS_->discount(t + dt, true) /
                                  (riskFreeTS_->discount(t + dt, true) * dq);
            const Real strikemt = strike * dr * dividendTS_->discount(t - dt, true) /
                                  (riskFreeTS_->discount(t - dt, true) * dq);
            const Real wpt =
                laterVariance(w, blackTS_->blackVariance(t + dt, strikept, true), strike, t, t + dt);
            Real wmt = blackTS_->blackVariance(t - dt, strikemt, true);
            if (laterVariance(wmt, w, strike, t - dt, t) == wmt)
                wmt = w;  // they differ only by rounding
            dwdt = (wpt - wmt) / (2.0 * dt);
        }

        // no variance accrues here, e.g. over a weekend in trading time
        if (dwdt <= 0.0)
            return minLocalVol_;

        if (dwdy == 0.0 && d2wdy2 == 0.0) // avoid /w where w might be 0.0
            return std::max<Volatility>(std::sqrt(dwdt), minLocalVol_);

        const Real den1 = 1.0 - y / w * dwdy;
        const Real den2 = 0.25 * (-0.25 - 1.0 / w + y * y / w / w) * dwdy * dwdy;
        const Real den3 = 0.5 * d2wdy2;
        const Real den = den1 + den2 + den3;
        QL_REQUIRE(den > 0.0, "butterfly arbitrage: non-positive density at strike "
                                  << strike << " and time " << t << " (Dupire denominator " << den
                                  << ")");
        return std::max<Volatility>(std::sqrt(dwdt / den), minLocalVol_);
    }

}
