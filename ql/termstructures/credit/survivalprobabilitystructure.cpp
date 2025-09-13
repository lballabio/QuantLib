/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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

#include <ql/termstructures/credit/survivalprobabilitystructure.hpp>

namespace QuantLib {

    SurvivalProbabilityStructure::SurvivalProbabilityStructure(
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(dc, jumps, jumpDates) {}

    SurvivalProbabilityStructure::SurvivalProbabilityStructure(
                                    const Date& refDate,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(refDate, cal, dc, jumps, jumpDates) {}

    SurvivalProbabilityStructure::SurvivalProbabilityStructure(
                                    Natural settlDays,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(settlDays, cal, dc, jumps, jumpDates) {}

    Real SurvivalProbabilityStructure::defaultDensityImpl(Time t) const {
        Time dt = 0.0001;
        Time t1 = std::max(t-dt, 0.0);
        Time t2 = t+dt;

        Probability p1 = survivalProbabilityImpl(t1);
        Probability p2 = survivalProbabilityImpl(t2);

        return (p1-p2)/(t2-t1);
    }

}
