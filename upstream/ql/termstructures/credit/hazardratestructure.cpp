/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
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

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/termstructures/credit/hazardratestructure.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        template <class F>
        struct remapper {
            F f;
            Time T;
            remapper(F f, Time T) : f(std::move(f)), T(T) {}
            // This remaps [-1,1] to [0,T]. No differential included.
            Real operator()(Real x) const {
                const Real arg = (x+1.0)*T/2.0;
                return f(arg);
            }
        };

        template <class F>
        remapper<F> remap(const F& f, Time T) {
            return remapper<F>(f,T);
        }

    }

    HazardRateStructure::HazardRateStructure(
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(dc, jumps, jumpDates) {}

    HazardRateStructure::HazardRateStructure(
                                    const Date& refDate,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(refDate, cal, dc, jumps, jumpDates) {}

    HazardRateStructure::HazardRateStructure(
                                    Natural settlDays,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(settlDays, cal, dc, jumps, jumpDates) {}

    Real HazardRateStructure::hazardRateImpl(Time) const {
        QL_FAIL("hazardRateImpl() must be implemented by a class derived from HazardRateStructure");
    }

    Probability HazardRateStructure::survivalProbabilityImpl(Time t) const {
        static GaussChebyshevIntegration integral(48);
        // the Gauss-Chebyshev quadratures integrate over [-1,1],
        // hence the remapping (and the Jacobian term t/2)
        return std::exp(-integral(remap([&](Time tau){ return hazardRateImpl(tau); }, t)) * t/2.0);
    }

}
