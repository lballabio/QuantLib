/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Hiroto Ogawa

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

#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/pricingengines/futures/discountingperpetualfuturesengine.hpp>

namespace QuantLib {

    DiscountingPerpetualFuturesEngine::DiscountingPerpetualFuturesEngine(
        Handle<YieldTermStructure> domesticDiscountCurve,
        Handle<YieldTermStructure> foreignDiscountCurve,
        Handle<Quote> assetSpot,
        Array& fundingTimes,
        Array& fundingRates,
        Array& interestRateDiffs,
        DiscountingPerpetualFuturesEngine::InterpolationType fundingInterpType)
    : domesticDiscountCurve_(std::move(domesticDiscountCurve)),
      foreignDiscountCurve_(std::move(foreignDiscountCurve)), assetSpot_(assetSpot),
      fundingTimes_(fundingTimes), fundingRates_(fundingRates),
      interestRateDiffs_(interestRateDiffs), fundingInterpType_(fundingInterpType) {
        registerWith(domesticDiscountCurve_);
        registerWith(foreignDiscountCurve_);
        registerWith(assetSpot_);
        QL_REQUIRE(fundingTimes_.size() > 0, "fundingTimes is empty");
        QL_REQUIRE(fundingRates_.size() > 0, "fundingRates is empty");
        QL_REQUIRE(interestRateDiffs_.size() > 0, "interestRateDiffs is empty");
        QL_REQUIRE(fundingTimes_.size() == fundingRates_.size(),
                   "fundingTimes and fundimgRates must have the same size.");
        QL_REQUIRE(fundingTimes_.size() == interestRateDiffs_.size(),
                   "fundingTimes and interestRateDiffs must have the same size.");
    }

    void DiscountingPerpetualFuturesEngine::calculate() const {
        QL_REQUIRE(!domesticDiscountCurve_.empty(),
                   "domestic discounting term structure handle is empty");
        QL_REQUIRE(!foreignDiscountCurve_.empty(),
                   "foreign discounting term structure handle is empty");
        QL_REQUIRE(!assetSpot_.empty(), "asset spot handle is empty");

        results_.value = 0.0;
        results_.errorEstimate = Null<Real>();

        Date refDate = domesticDiscountCurve_->referenceDate();
        Interpolation fundingRateInterp =
            DiscountingPerpetualFuturesEngine::selectInterpolation(fundingTimes_, fundingRates_);
        QL_REQUIRE(fundingRateInterp(fundingRateInterp.xMax()) > 0,
                   "fundingRate at max time is nagative. Because the last funding rate is flatly extrapolated, integral diverges.");
        Interpolation interestRateDiffInterp =
            DiscountingPerpetualFuturesEngine::selectInterpolation(fundingTimes_,
                                                                   interestRateDiffs_);

        Real timeIntegral = 0.;
        TrapezoidIntegral<Default> integrator(1.e-4, 20);
        Real fundingRate_xMax = fundingRateInterp.xMax();
        auto expIRDiff = [fundingRateInterp, integrator, fundingRate_xMax](Real s) {
            if (s < fundingRate_xMax) {
                return exp(-integrator(fundingRateInterp, 0., s));
            } else {
                return exp(-integrator(fundingRateInterp, 0., fundingRate_xMax)
                    - fundingRateInterp(fundingRate_xMax) * (s - fundingRate_xMax));
            }
        };

        auto timeIntegrand = [fundingRateInterp, interestRateDiffInterp, integrator, expIRDiff,
                              this](Real s) {
            return (fundingRateInterp(s) - interestRateDiffInterp(s)) * expIRDiff(s) *
                   this->foreignDiscountCurve_->discount(s) /
                   this->domesticDiscountCurve_->discount(s);
        };

        results_.value = assetSpot_->value() * integrator(timeIntegrand, 0., 30.);
    }

    Interpolation
    DiscountingPerpetualFuturesEngine::selectInterpolation(const Array& times,
                                                           const Array& values) const {
        Interpolation interpolator;
        switch (fundingInterpType_) {
            case Linear:
                interpolator = LinearInterpolation(times.begin(), times.end(), values.begin());
                break;
            case PiecewiseConstant:
                interpolator =
                    BackwardFlatInterpolation(times.begin(), times.end(), values.begin());
                break;
            case CubicSpline:
                interpolator = CubicNaturalSpline(times.begin(), times.end(), values.begin());
                break;
            default:
                QL_FAIL("Unknown interpolation type");
        }
        return interpolator;
    }

}
