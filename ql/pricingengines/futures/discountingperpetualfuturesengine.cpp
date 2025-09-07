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
        Handle<YieldTermStructure>& domesticDiscountCurve,
        Handle<YieldTermStructure>& foreignDiscountCurve,
        Handle<Quote>& assetSpot,
        Array& fundingTimes,
        Array& fundingRates,
        Array& interestRateDiffs,
        DiscountingPerpetualFuturesEngine::InterpolationType fundingInterpType)
    : domesticDiscountCurve_(std::move(domesticDiscountCurve)),
      foreignDiscountCurve_(std::move(foreignDiscountCurve)), assetSpot_(assetSpot),
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

        //Date refDate = domesticDiscountCurve_->referenceDate();
        //Array effFundingRates = fundingRates_;
        //Array effInterestRateDiffs = interestRateDiffs_;
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

        // discrete-time case
        if (fundingFreq.length() > 0) { 
            std::vector<Real> timeGrid;
            Real grid_time = 0.;
            while (grid_time < maxT_) {
                timeGrid.push_back(grid_time);
                Real unit_t = 0.;
                Date date = yearFractionToDate(dc, refDate, grid_time);
                Real daysInYear = dc.dayCount(Date(1, January, date.year()), Date(1, January, date.year()+1));
                switch (fundingFreq.units()) { 
                    case Years:
                    case Months:
                    case Weeks:
                    case Days:
                        grid_time = dc.yearFraction(refDate, cal.advance(date, fundingFreq));
                        break;
                    case Hours:
                        unit_t = 1. / daysInYear / 24.;
                        grid_time += unit_t * fundingFreq.length();
                        break;
                    case Minutes:
                        unit_t = 1. / daysInYear / 24. / 60.;
                        grid_time += unit_t * fundingFreq.length();
                        break;
                    case Seconds:
                        unit_t = 1. / daysInYear / 24. / 60. / 60.;
                        grid_time += unit_t * fundingFreq.length();
                        break;
                    case Milliseconds:
                        unit_t = 1. / daysInYear / 24. / 60. / 60. / 1000.;
                        grid_time += unit_t * fundingFreq.length();
                        break;
                    case Microseconds:
                        unit_t = 1. / daysInYear / 24. / 60. / 60. / 1000. / 1000.;
                        grid_time += unit_t * fundingFreq.length();
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
                    Real time = fundingTimes_[i];
                    Real nextTime = fundingTimes_[i+1];
                    ratio = foreignDiscountCurve_->discount(nextTime) /
                            foreignDiscountCurve_->discount(time) /
                            domesticDiscountCurve_->discount(nextTime) *
                            domesticDiscountCurve_->discount(time);
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
            for (Size i = 0; i < timeGrid.size(); ++i) {
                Real time = timeGrid[i];
                sum += productIRDiff(i) * (fundingRateGrid[i] - interestRateDiffGrid[i]) *
                       this->foreignDiscountCurve_->discount(time) /
                       this->domesticDiscountCurve_->discount(time);
                df_dom.push_back(this->domesticDiscountCurve_->discount(time));
                df_for.push_back(this->foreignDiscountCurve_->discount(time));
            }
            Size i_last = timeGrid.size() - 1;
            Real time_last = timeGrid[i_last];
            Real productIRDiff_last = productIRDiff(i_last);
            Real fundingRateGrid_last = fundingRateGrid[i_last];
            Real interestRateDiffGrid_last = interestRateDiffGrid[i_last];
            Real dt = 1.e-4;
            Real domRate_last = (log(this->domesticDiscountCurve_->discount(time_last)) -
                                log(this->domesticDiscountCurve_->discount(time_last + dt))) / dt;
            Real forRate_last = (log(this->foreignDiscountCurve_->discount(time_last)) -
                                 log(this->foreignDiscountCurve_->discount(time_last + dt))) / dt;
            // for remaining part, assume flat extrapolaiton on all rates
            // inside summation:
            Real last_term = productIRDiff_last *
                             (fundingRateGrid_last - interestRateDiffGrid_last) *
                             this->foreignDiscountCurve_->discount(time_last) /
                             this->domesticDiscountCurve_->discount(time_last);
            Real time_step = (timeGrid.back() - timeGrid.front()) / (timeGrid.size() - 1);
            Real ratio =
                1. / (1. + fundingRateGrid_last) * exp(-time_step * (forRate_last - domRate_last));
            sum += last_term / (1. - ratio);
            results_.value = assetSpot_->value() * sum;
        } else {
        // continuous-time case
            Real timeIntegral = 0.;
            TrapezoidIntegral<Default> integrator(1.e-4, 20);
            Real fundingRate_xMax = fundingRateInterp.xMax();
            auto expIRDiff = [fundingRateInterp, integrator, fundingRate_xMax](Real s) {
                if (s < fundingRate_xMax) {
                    return exp(-integrator(fundingRateInterp, 0., s));
                } else {
                    return exp(-integrator(fundingRateInterp, 0., fundingRate_xMax) -
                               fundingRateInterp(fundingRate_xMax) * (s - fundingRate_xMax));
                }
            };

            auto timeIntegrand = [fundingRateInterp, interestRateDiffInterp, integrator,
                                  expIRDiff, this](Real s) {
                return (fundingRateInterp(s) - interestRateDiffInterp(s)) * expIRDiff(s) *
                       this->foreignDiscountCurve_->discount(s) /
                       this->domesticDiscountCurve_->discount(s);
            };
            results_.value = assetSpot_->value() * integrator(timeIntegrand, 0., maxT_);
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
