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

#include <ql/experimental/fxslv/fxdeltaconverter.hpp>
#include <ql/experimental/fxslv/fxrootbracketing.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <cmath>

namespace QuantLib {

    FxDeltaConverter::FxDeltaConverter(DeltaVolQuote::DeltaType type,
                                       Real spot,
                                       DiscountFactor domesticDiscount,
                                       DiscountFactor foreignDiscount)
    : type_(type), spot_(spot), forward_(spot * foreignDiscount / domesticDiscount),
      domesticDiscount_(domesticDiscount), foreignDiscount_(foreignDiscount) {
        QL_REQUIRE(spot > 0.0, "positive spot required: " << spot);
        QL_REQUIRE(domesticDiscount > 0.0 && foreignDiscount > 0.0,
                   "positive discount factors required: " << domesticDiscount << ", "
                                                          << foreignDiscount);
    }

    BlackDeltaCalculator FxDeltaConverter::calculator(Option::Type parity, Real stdDev) const {
        return BlackDeltaCalculator(parity, type_, spot_, domesticDiscount_, foreignDiscount_,
                                    stdDev);
    }

    Real FxDeltaConverter::delta(Option::Type parity, Rate strike, Real stdDev) const {
        return calculator(parity, stdDev).deltaFromStrike(strike);
    }

    Rate FxDeltaConverter::atmStrike(DeltaVolQuote::AtmType type, Real stdDev) const {
        return calculator(Option::Call, stdDev).atmStrike(type);
    }

    Rate FxDeltaConverter::peakCallStrike(Real stdDev) const {
        QL_REQUIRE(premiumAdjusted(), "the call delta only peaks for premium-adjusted deltas");
        QL_REQUIRE(stdDev > 0.0, "positive standard deviation required: " << stdDev);
        // The premium-adjusted call delta is proportional to (K/F) N(d2);
        // its derivative in K vanishes where w x N(d2) = n(d2), w = stdDev.
        // h(d) = w x N(d) - n(d) falls to its minimum at d = -w, where it is
        // negative, and rises to w after it, so it has a single root above -w.
        const Real w = stdDev;
        CumulativeNormalDistribution N;
        NormalDistribution n;
        auto h = [&](Real d) { return w * N(d) - n(d); };
        const Real d2 = detail::findRoot(h, -w, 1.0, detail::Slope::Increasing, 1.0e-14);
        // d2 = log(F/K)/w - w/2
        return forward_ * std::exp(-w * d2 - 0.5 * w * w);
    }

    Rate FxDeltaConverter::strike(Option::Type parity, Real delta, Real stdDev) const {
        QL_REQUIRE(stdDev > 0.0, "positive standard deviation required: " << stdDev);
        QL_REQUIRE(delta * static_cast<Real>(parity) > 0.0,
                   "option type and delta are incoherent: " << parity << ", " << delta);

        if (!premiumAdjusted())
            return calculator(parity, stdDev).strikeFromDelta(delta);

        // Premium-adjusted deltas are solved for in log-strike.  Puts fall
        // monotonically from 0 to -infinity as the strike grows; calls
        // fall from their peak to 0 above the peak strike.
        auto error = [&](Real x) { return this->delta(parity, std::exp(x), stdDev) - delta; };

        Real start = std::log(forward_);
        if (parity == Option::Call) {
            const Rate kPeak = peakCallStrike(stdDev);
            const Real maxDelta = this->delta(Option::Call, kPeak, stdDev);
            QL_REQUIRE(delta <= maxDelta,
                       "call delta " << delta << " not attainable: the largest premium-adjusted "
                                     << "call delta at a standard deviation of " << stdDev << " is "
                                     << maxDelta);
            // the error is not negative at the peak, and falls above it
            start = std::log(kPeak);
        }
        return std::exp(detail::findRoot(error, start, stdDev, detail::Slope::Decreasing, 1.0e-12));
    }

    Real FxDeltaConverter::callMinusPutDelta(Rate strike) const {
        switch (type_) {
          case DeltaVolQuote::Spot:
            return foreignDiscount_;
          case DeltaVolQuote::Fwd:
            return 1.0;
          case DeltaVolQuote::PaSpot:
            return foreignDiscount_ * strike / forward_;
          case DeltaVolQuote::PaFwd:
            return strike / forward_;
          default:
            QL_FAIL("unknown delta type");
        }
    }

    Real FxDeltaConverter::callDeltaLimit() const {
        QL_REQUIRE(!premiumAdjusted(),
                   "the largest premium-adjusted call delta depends on the smile");
        return callMinusPutDelta(forward_);
    }

    

    

}
