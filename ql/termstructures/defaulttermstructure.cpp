/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
                                                         const DayCounter& dc)
    : TermStructure(dc) {}

    DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
                                                    const Date& referenceDate,
                                                    const Calendar& cal,
                                                    const DayCounter& dc)
    : TermStructure(referenceDate, cal, dc) {}

    DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
                                                         Natural settlementDays,
                                                         const Calendar& cal,
                                                         const DayCounter& dc)
    : TermStructure(settlementDays, cal, dc) {}


    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return 1.0 - survivalProbability(d, extrapolate);
    }

    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     Time t,
                                                     bool extrapolate) const {
        return 1.0 - survivalProbability(t, extrapolate);
    }

    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     const Date& d1,
                                                     const Date& d2,
                                                     bool extrapolate) const {
        QL_REQUIRE(d1 <= d2,
                   "initial date (" << d1 << ") "
                   "later than final date (" << d2 << ")");
        Probability p1 = defaultProbability(d1,extrapolate),
                    p2 = defaultProbability(d2,extrapolate);
        return p2 - p1;
    }

    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     Time t1,
                                                     Time t2,
                                                     bool extrapolate) const {
        QL_REQUIRE(t1 <= t2,
                   "initial time (" << t1 << ") "
                   "later than final time (" << t2 << ")");
        Probability p1 = defaultProbability(t1,extrapolate),
                    p2 = defaultProbability(t2,extrapolate);
        return p2 - p1;
    }


    Probability DefaultProbabilityTermStructure::survivalProbability(
                                                     const Date& d,
                                                     bool extrapolate) const {
        checkRange(d, extrapolate);
        return survivalProbabilityImpl(timeFromReference(d));
    }

    Probability DefaultProbabilityTermStructure::survivalProbability(
                                                     Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);
        return survivalProbabilityImpl(t);
    }


    Real DefaultProbabilityTermStructure::defaultDensity(
                                                     const Date& d,
                                                     bool extrapolate) const {
        checkRange(d, extrapolate);
        return defaultDensityImpl(timeFromReference(d));
    }

    Real DefaultProbabilityTermStructure::defaultDensity(
                                                     Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);
        return defaultDensityImpl(t);
    }


    Real DefaultProbabilityTermStructure::hazardRate(const Date& d,
                                                     bool extrapolate) const {
        checkRange(d, extrapolate);
        return hazardRateImpl(timeFromReference(d));
    }

    Real DefaultProbabilityTermStructure::hazardRate(Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);
        return hazardRateImpl(t);
    }

}

