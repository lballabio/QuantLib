/*
 Copyright (C) 2019 Quaternion Risk Management Ltd
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)
 Copyright (C) 2025 Paolo D'Elia
 All rights reserved.

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
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/fx/blackdeltacalculator.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/equityfx/blackvolsurfacedelta.hpp>
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/termstructures/volatility/flatsmilesection.hpp>

using namespace std;

namespace QuantLib {

    BlackVolatilitySurfaceDelta::BlackVolatilitySurfaceDelta(
        Date referenceDate, const std::vector<Date>& dates, const std::vector<Real>& putDeltas,
        const std::vector<Real>& callDeltas, bool hasAtm, const Matrix& blackVolMatrix, const DayCounter& dayCounter,
        const Calendar& cal, const Handle<Quote>& spot, const Handle<YieldTermStructure>& domesticTS,
        const Handle<YieldTermStructure>& foreignTS, DeltaVolQuote::DeltaType dt, DeltaVolQuote::AtmType at,
        ext::optional<DeltaVolQuote::DeltaType> atmDeltaType, const Period& switchTenor, DeltaVolQuote::DeltaType ltdt,
        DeltaVolQuote::AtmType ltat, ext::optional<DeltaVolQuote::DeltaType> longTermAtmDeltaType,
        SmileInterpolationMethod im, bool flatStrikeExtrapolation, BlackVolTimeExtrapolation timeExtrapolation)
        : BlackVolatilityTermStructure(referenceDate, cal, Following, dayCounter), dates_(dates), times_(dates.size(), 0),
        putDeltas_(putDeltas), callDeltas_(callDeltas), hasAtm_(hasAtm), spot_(spot), domesticTS_(domesticTS),
        foreignTS_(foreignTS), dt_(dt), at_(at), atmDeltaType_(atmDeltaType), switchTenor_(switchTenor), ltdt_(ltdt),
        ltat_(ltat), longTermAtmDeltaType_(longTermAtmDeltaType), interpolationMethod_(im),
        flatStrikeExtrapolation_(flatStrikeExtrapolation), timeExtrapolation_(timeExtrapolation) {

        // If ATM delta type is not given, set it to dt
        if (!atmDeltaType_)
            atmDeltaType_ = dt_;
        if (!longTermAtmDeltaType_)
            longTermAtmDeltaType_ = ltdt_;

        // set switch time
        switchTime_ = switchTenor_ == 0 * Days ? QL_MAX_REAL : timeFromReference(optionDateFromTenor(switchTenor));

        QL_REQUIRE(dates.size() > 1, "at least 1 date required");
        // this has already been done for dates
        for (Size i = 0; i < dates.size(); i++) {
            QL_REQUIRE(referenceDate < dates[i], "Dates must be greater than reference date");
            times_[i] = timeFromReference(dates[i]);
            if (i > 0) {
                QL_REQUIRE(times_[i] > times_[i - 1], "dates must be sorted unique!");
            }
        }

        // check size of matrix
        Size n = putDeltas.size() + (hasAtm ? 1 : 0) + callDeltas.size();
        QL_REQUIRE(n > 0, "Need at least one delta");
        QL_REQUIRE(blackVolMatrix.columns() == n, "Invalid number of columns in blackVolMatrix, got "
                                                    << blackVolMatrix.columns() << " but have " << n << " deltas");
        QL_REQUIRE(blackVolMatrix.rows() == dates.size(), "Invalid number of rows in blackVolMatrix, got "
                                                            << blackVolMatrix.rows() << " but have " << dates.size()
                                                            << " dates");

        // build interpolators for each delta
        // TODO: template this so it can be changed
        bool forceMonotoneVariance = false;
        for (Size i = 0; i < n; i++) {
            vector<Volatility> vols(dates.size());
            for (Size j = 0; j < dates.size(); j++) {
                vols[j] = blackVolMatrix[j][i];
            }

            // BlackVarianceCurve will make a local copy of vols and dates
            interpolators_.push_back(ext::make_shared<BlackVarianceCurve>(
                referenceDate, dates, vols, dayCounter, forceMonotoneVariance,
                timeExtrapolation));
        }

        // register
        registerWith(spot_);
        registerWith(domesticTS_);
        registerWith(foreignTS_);
    }

    ext::shared_ptr<SmileSection> BlackVolatilitySurfaceDelta::blackVolSmile(Time t) const {

        Real spot = spot_->value();
        DiscountFactor dDiscount = domesticTS_->discount(t);
        DiscountFactor fDiscount = foreignTS_->discount(t);
        Real sqrtT = sqrt(t);

        DeltaVolQuote::AtmType at;
        DeltaVolQuote::DeltaType dt;
        DeltaVolQuote::DeltaType atmDt;
        if (t < switchTime_ && !close_enough(t, switchTime_)) {
            at = at_;
            dt = dt_;
            atmDt = *atmDeltaType_;
        } else {
            at = ltat_;
            dt = ltdt_;
            atmDt = *longTermAtmDeltaType_;
        }

        // Store smile section in map. Use strikes as key and vols as values for automatic sorting by strike.
        // If we have already have a strike from a previous delta, we do not overwrite it.
        auto comp = [](Real a, Real b) { return !close(a, b) && a < b; };
        map<Real, Real, decltype(comp)> smileSection(comp);
        Size i = 0;
        Real atmLevel = 1.0; // set atmLevel to 1.0 in case hasAtm_ is false

        for (Real delta : putDeltas_) {
            Real vol = interpolators_.at(i)->blackVol(t, 1, true);
            try {
                BlackDeltaCalculator bdc(Option::Put, dt, spot, dDiscount, fDiscount, vol * sqrtT);
                Real strike = bdc.strikeFromDelta(delta);
                if (smileSection.count(strike) == 0)
                    smileSection[strike] = vol;
            } catch (const std::exception& e) {
                QL_FAIL("BlackVolatilitySurfaceDelta: Error during calculating put strike at delta " << delta << ": "
                                                                                                    << e.what());
            }
            i++;
        }
        if (hasAtm_) {
            Real vol = interpolators_.at(i)->blackVol(t, 1, true);
            atmLevel = vol;
            try {
                BlackDeltaCalculator bdc(Option::Put, atmDt, spot, dDiscount, fDiscount, vol * sqrtT);
                Real strike = bdc.atmStrike(at);
                if (smileSection.count(strike) == 0)
                    smileSection[strike] = vol;
            } catch (const std::exception& e) {
                QL_FAIL("BlackVolatilitySurfaceDelta: Error during calculating atm strike: " << e.what());
            }
            i++;
        }
        for (Real delta : callDeltas_) {
            Real vol = interpolators_.at(i)->blackVol(t, 1, true);
            try {
                BlackDeltaCalculator bdc(Option::Call, dt, spot, dDiscount, fDiscount, vol * sqrtT);
                Real strike = bdc.strikeFromDelta(delta);
                if (smileSection.count(strike) == 0)
                    smileSection[strike] = vol;
            } catch (const std::exception& e) {
                QL_FAIL("BlackVolatilitySurfaceDelta: Error during calculating call strike at delta " << delta << ": "
                                                                                                    << e.what());
            }
            i++;
        }

        // sort and extract to vectors
        vector<Real> strikes;
        strikes.reserve(smileSection.size());
        vector<Real> stdDevs;
        stdDevs.reserve(smileSection.size());
        for (const auto& kv : smileSection) {
            strikes.push_back(kv.first);
            stdDevs.push_back(kv.second * sqrtT);
        }

        // now build smile from strikes and vols
        QL_REQUIRE(!stdDevs.empty(),
                "BlackVolatilitySurfaceDelta::blackVolSmile(" << t << "): no strikes given, this is unexpected.");
        if (stdDevs.size() == 1) {
            // handle the situation that we only have one strike (might occur for e.g. t=0)
            return ext::make_shared<FlatSmileSection>(t, stdDevs.front() / sqrtT, dayCounter());
        } else {
            // we have at least two strikes
            if (interpolationMethod_ == SmileInterpolationMethod::Linear)
                return ext::make_shared<InterpolatedSmileSection<Linear>>(t, strikes, stdDevs, atmLevel, Linear(), dayCounter(), 
                                                                          VolatilityType::ShiftedLognormal, 0.0, flatStrikeExtrapolation_);
            else if (interpolationMethod_ == SmileInterpolationMethod::NaturalCubic)
                return ext::make_shared<InterpolatedSmileSection<Cubic>>(t, strikes, stdDevs, atmLevel, Cubic(CubicInterpolation::Kruger), 
                                                                         dayCounter(), VolatilityType::ShiftedLognormal, 0.0, 
                                                                         flatStrikeExtrapolation_);
            else if (interpolationMethod_ == SmileInterpolationMethod::FinancialCubic)
                return ext::make_shared<InterpolatedSmileSection<Cubic>>(t, 
                        strikes, 
                        stdDevs, 
                        atmLevel, 
                        Cubic(CubicInterpolation::Kruger, true, CubicInterpolation::SecondDerivative, 0.0,
                                    CubicInterpolation::FirstDerivative),
                        dayCounter(),
                        VolatilityType::ShiftedLognormal, 
                        0.0, 
                        flatStrikeExtrapolation_);
            else if (interpolationMethod_ == SmileInterpolationMethod::CubicSpline)
                return ext::make_shared<InterpolatedSmileSection<Cubic>>(t, strikes, stdDevs, atmLevel, Cubic(CubicInterpolation::Spline),
                                                                         dayCounter(), VolatilityType::ShiftedLognormal, 0.0, 
                                                                         flatStrikeExtrapolation_);
            else {
                QL_FAIL("Invalid method " << (int)interpolationMethod_);
            }
        }
    }

    ext::shared_ptr<SmileSection> BlackVolatilitySurfaceDelta::blackVolSmile(const Date& d) const {
        return blackVolSmile(timeFromReference(d));
    }

    Real BlackVolatilitySurfaceDelta::forward(Time t) const {
        return spot_->value() * foreignTS_->discount(t) / domesticTS_->discount(t); // TODO
    }

    Volatility BlackVolatilitySurfaceDelta::blackVolImpl(Time t, Real strike) const {
        // If asked for strike == 0, just return the ATM value.
        double tme =
            (t > times_.back() && timeExtrapolation_ == BlackVolTimeExtrapolation::FlatVolatility) ? times_.back() : t;

        if (strike == 0 || strike == Null<Real>()) {
            if (hasAtm_) {
                // ask the ATM interpolator directly
                return interpolators_[putDeltas_.size()]->blackVol(tme, Null<Real>(), true);
            } else {
                // set strike to be fwd and we will return ATMF
                strike = forward(tme);
            }
        }
        return blackVolSmile(tme)->volatility(strike);
    }

} // namespace QuantLib
