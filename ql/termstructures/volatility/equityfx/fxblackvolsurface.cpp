/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <qle/termstructures/fxblackvolsurface.hpp>

using namespace std;

namespace QuantExt {

FxBlackVolatilitySurface::FxBlackVolatilitySurface(const Date& referenceDate, const std::vector<Date>& dates,
                                                   const std::vector<Volatility>& atmVols,
                                                   const std::vector<Volatility>& rr25d,
                                                   const std::vector<Volatility>& bf25d, const DayCounter& dayCounter,
                                                   const Calendar& cal, const Handle<Quote>& fxSpot,
                                                   const Handle<YieldTermStructure>& domesticTS,
                                                   const Handle<YieldTermStructure>& foreignTS)
    : BlackVolatilityTermStructure(referenceDate, cal), times_(dates.size()), dayCounter_(dayCounter), fxSpot_(fxSpot),
      domesticTS_(domesticTS), foreignTS_(foreignTS), atmCurve_(referenceDate, dates, atmVols, dayCounter),
      rr25d_(rr25d), bf25d_(bf25d) {

    QL_REQUIRE(dates.size() > 1, "at least 1 date required");
    maxDate_ = dates.back();

    QL_REQUIRE(dates.size() == rr25d.size(), "mismatch between date vector and 25D RR vector");
    QL_REQUIRE(dates.size() == bf25d.size(), "mismatch between date vector and 25D BF vector");

    // this has already been done for dates
    for (Size i = 0; i < dates.size(); i++) {
        QL_REQUIRE(referenceDate < dates[i], "Dates must be greater than reference date");
        times_[i] = timeFromReference(dates[i]);
        if (i > 0) {
            QL_REQUIRE(times_[i] > times_[i - 1], "dates must be sorted unique!");
        }
    }

    // set up the 3 interpolators
    rrCurve_ = LinearInterpolation(times_.begin(), times_.end(), rr25d_.begin());
    bfCurve_ = LinearInterpolation(times_.begin(), times_.end(), bf25d_.begin());

    atmCurve_.enableExtrapolation();

    registerWith(domesticTS_);
    registerWith(foreignTS_);
    registerWith(fxSpot_);
}

boost::shared_ptr<FxSmileSection> FxBlackVolatilitySurface::blackVolSmile(Time t) const {
    // we interpolate on the 3 curves independently
    Volatility atm = atmCurve_.blackVol(t, 0); // any strike will do

    // Flat extrapolation for RR + BF.
    Volatility rr, bf;
    if (t < times_.front()) {
        // flat RR + BF
        rr = rrCurve_(times_.front());
        bf = bfCurve_(times_.front());
    } else if (t < times_.back()) {
        rr = rrCurve_(t, true);
        bf = bfCurve_(t, true);
    } else {
        // flat RR + BF
        rr = rrCurve_(times_.back());
        bf = bfCurve_(times_.back());
    }

    Real rd = domesticTS_->zeroRate(t, Continuous);
    Real rf = foreignTS_->zeroRate(t, Continuous);

    return blackVolSmileImpl(fxSpot_->value(), rd, rf, t, atm, rr, bf);
}

Volatility FxBlackVolatilitySurface::blackVolImpl(Time t, Real strike) const {
    // If asked for strike == 0, just return the ATM value.
    if (strike == 0 || strike == Null<Real>())
        return atmCurve_.blackVol(t, 0);
    else
        return blackVolSmile(t)->volatility(strike);
}

} // namespace QuantExt
