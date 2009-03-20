/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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

#include <ql/termstructures/credit/survivalprobabilitystructure.hpp>

namespace QuantLib {

    SurvivalProbabilityStructure::SurvivalProbabilityStructure(
                                                         const DayCounter& dc)
    : DefaultProbabilityTermStructure(dc) {}

    SurvivalProbabilityStructure::SurvivalProbabilityStructure(
                                                    const Date& referenceDate,
                                                    const Calendar& cal,
                                                    const DayCounter& dc)
    : DefaultProbabilityTermStructure(referenceDate, cal, dc) {}

    SurvivalProbabilityStructure::SurvivalProbabilityStructure(
                                                        Natural settlementDays,
                                                        const Calendar& cal,
                                                        const DayCounter& dc)
    : DefaultProbabilityTermStructure(settlementDays, cal, dc) {}


    Real SurvivalProbabilityStructure::hazardRateImpl(Time t) const {
        Probability S = survivalProbabilityImpl(t);
        return S == 0.0 ? 0.0 : defaultDensityImpl(t)/S;
    }

    Real SurvivalProbabilityStructure::defaultDensityImpl(Time t) const {
        Time dt = 0.0001;
        Time t1 = std::max(t-dt, 0.0);
        Time t2 = t+dt;

        Probability S1 = survivalProbabilityImpl(t1);
        Probability S2 = survivalProbabilityImpl(t2);

        return -(S2-S1)/(t2-t1);
    }

}
