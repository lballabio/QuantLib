/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/blackvolsurface.hpp>

namespace QuantLib {

    BlackVolSurface::BlackVolSurface(const DayCounter& dc)
    : BlackAtmVolCurve(dc) {}

    BlackVolSurface::BlackVolSurface(const Date& refDate,
                                     const Calendar& cal,
                                     const DayCounter& dc)
    : BlackAtmVolCurve(refDate, cal, dc) {}

    BlackVolSurface::BlackVolSurface(Natural settlDays,
                                     const Calendar& cal,
                                     const DayCounter& dc)
    : BlackAtmVolCurve(settlDays, cal, dc) {}

    Volatility BlackVolSurface::volatility(const Date& maturity,
                                           Real strike,
                                           bool extrapolate) const {
        Time t = timeFromReference(maturity);
        checkRange(t, strike, extrapolate);
        return volImpl(t, strike);
    }

    Volatility BlackVolSurface::volatility(Time maturity,
                                           Real strike,
                                           bool extrapolate) const {
        checkRange(maturity, strike, extrapolate);
        return volImpl(maturity, strike);
    }

    Real BlackVolSurface::variance(const Date& maturity,
                                   Real strike,
                                   bool extrapolate) const {
        Time t = timeFromReference(maturity);
        checkRange(t, strike, extrapolate);
        return varianceImpl(t, strike);
    }

    Real BlackVolSurface::variance(Time maturity,
                                   Real strike,
                                   bool extrapolate) const {
        checkRange(maturity, strike, extrapolate);
        return varianceImpl(maturity, strike);
    }


    Real BlackVolSurface::atmVarianceImpl(Time t) const {
        return varianceImpl(t, atmLevel(t));
    }

    Volatility BlackVolSurface::atmVolImpl(Time t) const {
        return volImpl(t, atmLevel(t));
    }

    void BlackVolSurface::accept(AcyclicVisitor& v) {
        Visitor<BlackVolSurface>* v1 =
            dynamic_cast<Visitor<BlackVolSurface>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not a BlackVolSurface term structure visitor");
    }

    void BlackVolSurface::checkRange(const Date& d,
                                     Real k,
                                     bool extrapolate) const {
        TermStructure::checkRange(d, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

    void BlackVolSurface::checkRange(Time t,
                                     Real k,
                                     bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}
