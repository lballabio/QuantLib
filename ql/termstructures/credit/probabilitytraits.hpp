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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file probabilitytraits.hpp
    \brief default-probability bootstrap traits
*/

#ifndef ql_probability_traits_hpp
#define ql_probability_traits_hpp

#include <ql/termstructures/credit/interpolatedsurvivalprobabilitycurve.hpp>
#include <ql/termstructures/credit/interpolatedhazardratecurve.hpp>
#include <ql/termstructures/credit/interpolateddefaultdensitycurve.hpp>
#include <ql/termstructures/bootstraphelper.hpp>

namespace QuantLib {

    namespace detail {
        const Rate avgHazardRate = 0.01;
    }

    //! Survival-Probability-curve traits
    struct SurvivalProbability {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedSurvivalProbabilityCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<DefaultProbabilityTermStructure> helper;
        // start of curve data
        static Date initialDate(const DefaultProbabilityTermStructure* c) {
            return c->referenceDate();
        }
        // value at reference date
        static Real initialValue(const DefaultProbabilityTermStructure*) {
            return 1.0;
        }
        // true if the initialValue is just a dummy value
        static bool dummyInitialValue() { return false; }
        // initial guess
        static Real initialGuess() {
            return 1.0/(1.0+detail::avgHazardRate*0.25);
        }
        // further guesses
        static Real guess(const DefaultProbabilityTermStructure* c,
                          const Date& d) {
            return c->survivalProbability(d,true);
        }
        // possible constraints based on previous values
        static Real minValueAfter(Size,
                                  const std::vector<Real>&) {
            return QL_EPSILON;
        }
        static Real maxValueAfter(Size i,
                                  const std::vector<Real>& data) {
            return data[i-1];
        }
        // update with new guess
        static void updateGuess(std::vector<Real>& data,
                                Probability p,
                                Size i) {
            data[i] = p;
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 50; }
    };

    //! Hazard-rate-curve traits
    struct HazardRate {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedHazardRateCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<DefaultProbabilityTermStructure> helper;
        // start of curve data
        static Date initialDate(const DefaultProbabilityTermStructure* c) {
            return c->referenceDate();
        }
        // dummy value at reference date
        static Real initialValue(const DefaultProbabilityTermStructure*) {
            return detail::avgHazardRate;
        }
        // true if the initialValue is just a dummy value
        static bool dummyInitialValue() { return true; }
        // initial guess
        static Real initialGuess() { return detail::avgHazardRate; }
        // further guesses
        static Real guess(const DefaultProbabilityTermStructure* c,
                          const Date& d) {
            return c->hazardRate(d, true);
        }
        // possible constraints based on previous values
        static Real minValueAfter(Size,
                                  const std::vector<Real>&) {
            return QL_EPSILON;
        }
        static Real maxValueAfter(Size,
                                  const std::vector<Real>&) {
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return 200.0;
        }
        // update with new guess
        static void updateGuess(std::vector<Real>& data,
                                Real rate,
                                Size i) {
            data[i] = rate;
            if (i == 1)
                data[0] = rate; // first point is updated as well
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 30; }
    };

    //! Default-density-curve traits
    struct DefaultDensity {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedDefaultDensityCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<DefaultProbabilityTermStructure> helper;
        // start of curve data
        static Date initialDate(const DefaultProbabilityTermStructure* c) {
            return c->referenceDate();
        }
        // value at reference date
        static Real initialValue(const DefaultProbabilityTermStructure*) {
            return detail::avgHazardRate;
        }
        // true if the initialValue is just a dummy value
        static bool dummyInitialValue() { return true; }
        // initial guess
        static Real initialGuess() { return detail::avgHazardRate; }
        // further guesses
        static Real guess(const DefaultProbabilityTermStructure* c,
                          const Date& d) {
            return c->defaultDensity(d, true);
        }
        // possible constraints based on previous values
        static Real minValueAfter(Size,
                                  const std::vector<Real>&) {
            return QL_EPSILON;
        }
        static Real maxValueAfter(Size,
                                  const std::vector<Real>&) {
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return 3.0;
        }
        // update with new guess
        static void updateGuess(std::vector<Real>& data,
                                Real density,
                                Size i) {
            data[i] = density;
            if (i == 1)
                data[0] = density; // first point is updated as well
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 30; }
    };

}


#endif
