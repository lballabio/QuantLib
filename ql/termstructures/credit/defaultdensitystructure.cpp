/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
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

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/termstructures/credit/defaultdensitystructure.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        template <class F>
        struct t_remapper {
            F f;
            Time T;
            t_remapper(F f, Time T) : f(std::move(f)), T(T) {}
            // This remaps [-1,1] to [0,T]. No differential included.
            Real operator()(Real x) const {
                const Real arg = (x+1.0)*T/2.0;
                return f(arg);
            }
        };

        template <class F>
        t_remapper<F> remap_t(const F& f, Time T) {
            return t_remapper<F>(f,T);
        }

    }

    DefaultDensityStructure::DefaultDensityStructure(
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(dc, jumps, jumpDates) {}

    DefaultDensityStructure::DefaultDensityStructure(
                                    const Date& refDate,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(refDate, cal, dc, jumps, jumpDates) {}

    DefaultDensityStructure::DefaultDensityStructure(
                                    Natural settlDays,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(settlDays, cal, dc, jumps, jumpDates) {}

    Probability DefaultDensityStructure::survivalProbabilityImpl(Time t) const {
        static GaussChebyshevIntegration integral(48);
        // the Gauss-Chebyshev quadratures integrate over [-1,1],
        // hence the remapping (and the Jacobian term t/2)
        Probability P = 1.0 - integral(remap_t([&](Time tau){ return defaultDensityImpl(tau); }, t)) * t / 2.0;
        //QL_ENSURE(P >= 0.0, "negative survival probability");
        return std::max<Real>(P, 0.0);
    }

}
