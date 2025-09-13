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
#include <ql/time/daycounters/yearfractiontodate.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    DiscountingPerpetualFuturesEngine::DiscountingPerpetualFuturesEngine(
        const Handle<YieldTermStructure>& domesticDiscountCurve,
        const Handle<YieldTermStructure>& foreignDiscountCurve,
        const Handle<Quote>& assetSpot,
        const Array fundingTimes,
        const Array fundingRates,
        const Array interestRateDiffs,
        DiscountingPerpetualFuturesEngine::InterpolationType fundingInterpType)
    : domesticDiscountCurve_(domesticDiscountCurve),
      foreignDiscountCurve_(foreignDiscountCurve), assetSpot_(assetSpot),
      fundingTimes_(fundingTimes), fundingRates_(fundingRates),
      interestRateDiffs_(interestRateDiffs), fundingInterpType_(fundingInterpType), maxT_(60.) {
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

        QL_REQUIRE(
            arguments_.payoffType == PerpetualFutures::Linear ||
                arguments_.payoffType == PerpetualFutures::Inverse,
            "Only Linear and Inverse payoffs are supported in DiscountingPerpetualFuturesEngine");

        // Linear payoff <--> Inverse payoff:
        // 1. exchange domestic and foreign curves
        // 2. future price: f <--> 1/f
        auto effDomCurve = arguments_.payoffType == PerpetualFutures::Linear ?
                               domesticDiscountCurve_ : foreignDiscountCurve_;
        auto effForCurve = arguments_.payoffType == PerpetualFutures::Linear ?
                               foreignDiscountCurve_ : domesticDiscountCurve_;

        Period fundingFreq = arguments_.fundingFrequency;
        Date refDate = Settings::instance().evaluationDate();
        DayCounter dc = arguments_.dc;
        Calendar cal = arguments_.cal;
        
        Interpolation fundingRateInterp =
            DiscountingPerpetualFuturesEngine::selectInterpolation(fundingTimes_, fundingRates_);
        fundingRateInterp.enableExtrapolation();
        QL_REQUIRE(fundingRateInterp(fundingRateInterp.xMax()) > 0,
                   "fundingRate at max time is nagative. Because the last funding rate is "
                   "flatly extrapolated, integral diverges.");
        Interpolation interestRateDiffInterp =
            DiscountingPerpetualFuturesEngine::selectInterpolation(fundingTimes_,
                                                                   interestRateDiffs_);
        interestRateDiffInterp.enableExtrapolation();

        Real factor = 0.;
        // discrete-time case
        if (fundingFreq.length() > 0) { 
            std::vector<Real> timeGrid;
            Real tGrid = 0.;
            while (tGrid < maxT_) {
                timeGrid.push_back(tGrid);
                Real tUnit = 0.;
                Date date = yearFractionToDate(dc, refDate, tGrid);
                Real daysInYear = dc.dayCount(Date(1, January, date.year()), Date(1, January, date.year()+1));
                switch (fundingFreq.units()) { 
                    case Years:
                        tGrid += fundingFreq.length();
                        break;
                    case Months:
                        tUnit = 1. / 12.;
                        tGrid += tUnit * fundingFreq.length();
                        break;
                    case Weeks:
                    case Days:
                        tGrid = dc.yearFraction(refDate, cal.advance(date, fundingFreq));
                        break;
                    case Hours:
                        tUnit = 1. / daysInYear / 24.;
                        tGrid += tUnit * fundingFreq.length();
                        break;
                    case Minutes:
                        tUnit = 1. / daysInYear / 24. / 60.;
                        tGrid += tUnit * fundingFreq.length();
                        break;
                    case Seconds:
                        tUnit = 1. / daysInYear / 24. / 60. / 60.;
                        tGrid += tUnit * fundingFreq.length();
                        break;
                    case Milliseconds:
                        tUnit = 1. / daysInYear / 24. / 60. / 60. / 1000.;
                        tGrid += tUnit * fundingFreq.length();
                        break;
                    case Microseconds:
                        tUnit = 1. / daysInYear / 24. / 60. / 60. / 1000. / 1000.;
                        tGrid += tUnit * fundingFreq.length();
                        break;
                    default:
                        QL_FAIL("Unknown unit in fundingFrequency");
                }
            }
            Array fundingRateGrid(timeGrid.size());
            Array interestRateDiffGrid(timeGrid.size());
            for (Size i = 0; i < timeGrid.size(); ++i) {
                Real time = timeGrid[i];
                fundingRateGrid[i] = fundingRateInterp(time);
                interestRateDiffGrid[i] = interestRateDiffInterp(time);
            }
            
            if (arguments_.fundingType == PerpetualFutures::AHJ_alt) {
                Real ratio = 1.;
                Size i;
                for (i = 0; i < timeGrid.size() - 1; ++i) {
                    Real time = timeGrid[i];
                    Real nextTime = timeGrid[i + 1];
                    ratio = effForCurve->discount(nextTime) / effForCurve->discount(time)
                          / effDomCurve->discount(nextTime) * effDomCurve->discount(time);
                    fundingRateGrid[i] *= ratio;
                    interestRateDiffGrid[i] *= ratio;
                }
                // for i = timeGrid.size() - 1
                fundingRateGrid[i] *= ratio;
                interestRateDiffGrid[i] *= ratio;
            }
            auto productIRDiff = [timeGrid, fundingRateGrid](Size i) {
                Real ret = 1.;
                for (Size j = 0; j <= i; ++j) {
                    ret /= 1. + fundingRateGrid[j];
                }
                return ret;
            };
            Real sum = 0.;
            std::vector<Real> df_dom, df_for;
            for (Size i = 0; i < timeGrid.size() - 1; ++i) {
                Real time = timeGrid[i];
                sum += productIRDiff(i) * (fundingRateGrid[i] - interestRateDiffGrid[i])
                       * effForCurve->discount(time) / effDomCurve->discount(time);
                df_dom.push_back(effDomCurve->discount(time));
                df_for.push_back(effForCurve->discount(time));
            }
            Size iLast = timeGrid.size() - 1;
            Real timeLast = timeGrid[iLast];
            Real productIRDiffLast = productIRDiff(iLast);
            Real fundingRateGridLast = fundingRateGrid[iLast];
            Real interestRateDiffGridLast = interestRateDiffGrid[iLast];
            Real dt = 1.e-4;
            Real domRateLast = (log(effDomCurve->discount(timeLast)) - log(effDomCurve->discount(timeLast + dt))) / dt;
            Real forRateLast = (log(effForCurve->discount(timeLast)) - log(effForCurve->discount(timeLast + dt))) / dt;

            // for t > maxT_, assume flat extrapolaiton on all rates
            Real lastTerm = productIRDiffLast
                             * (fundingRateGridLast - interestRateDiffGridLast)
                             * effForCurve->discount(timeLast) / effDomCurve->discount(timeLast);
            Real timeStep = (timeGrid.back() - timeGrid.front()) / (timeGrid.size() - 1);
            Real ratio =
                1. / (1. + fundingRateGridLast) * exp(-timeStep * (forRateLast - domRateLast));
            sum += lastTerm / (1. - ratio);
            factor = sum;

        } else {
        // continuous-time case
            Real timeIntegral = 0.;
            TrapezoidIntegral<Default> integrator(1.e-6, 30);
            Real fundingRateXMax = fundingRateInterp.xMax();
            auto expIRDiff = [fundingRateInterp, integrator, fundingRateXMax](Real s) {
                if (s < fundingRateXMax) {
                    return exp(-integrator(fundingRateInterp, 0., s));
                } else {
                    return exp(-integrator(fundingRateInterp, 0., fundingRateXMax) -
                               fundingRateInterp(fundingRateXMax) * (s - fundingRateXMax));
                }
            };

            auto timeIntegrand = [fundingRateInterp, interestRateDiffInterp, integrator, expIRDiff,
                                  effDomCurve, effForCurve](Real s) {
                return (fundingRateInterp(s) - interestRateDiffInterp(s)) * expIRDiff(s)
                       * effForCurve->discount(s) / effDomCurve->discount(s);
            };
            factor = integrator(timeIntegrand, 0., maxT_);

            // for t > maxT_, assume flat extrapolaiton on all rates
            Real fundingRateLast = fundingRateInterp(maxT_);
            Real interestRateDiffLast = interestRateDiffInterp(maxT_);
            Real expIRDiff_last = expIRDiff(maxT_);
            Real dt = 1.e-4;
            Real domRateLast = (log(effDomCurve->discount(maxT_)) - log(effDomCurve->discount(maxT_ + dt))) / dt;
            Real forRateLast = (log(effForCurve->discount(maxT_)) - log(effForCurve->discount(maxT_ + dt))) /dt;
            Real ratio = fundingRateLast + forRateLast - domRateLast;
            factor += (fundingRateLast - interestRateDiffLast) * expIRDiff_last *
                      effForCurve->discount(maxT_) / effDomCurve->discount(maxT_) / ratio;
        }

        if (arguments_.payoffType == PerpetualFutures::Linear) {
            results_.value = assetSpot_->value() * factor;
        } else {
            results_.value = assetSpot_->value() / factor;
        }
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
