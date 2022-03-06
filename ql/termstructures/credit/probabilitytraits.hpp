/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2016 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009, 2011 Ferdinando Ametrano

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
        const Real avgHazardRate = 0.01;
        const Real maxHazardRate = 1.0;
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

        // guesses
        template <class C>
        static Real guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return 1.0/(1.0+detail::avgHazardRate*0.25);

            // extrapolate
            Date d = c->dates()[i];
            return c->survivalProbability(d,true);
        }
        // constraints
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                return c->data().back()/2.0;
            }
            Time dt = c->times()[i] - c->times()[i-1];
            return c->data()[i-1] * std::exp(- detail::maxHazardRate * dt);
        }
        template <class C>
        static Real maxValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            // survival probability cannot increase
            return c->data()[i-1];
        }

        // root-finding update
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

        // guesses
        template <class C>
        static Real guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return detail::avgHazardRate;

            // extrapolate
            Date d = c->dates()[i];
            return c->hazardRate(d, true);
        }

        // constraints
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::min_element(c->data().begin(), c->data().end()));
                return r/2.0;
            }
            return QL_EPSILON;
        }
        template <class C>
        static Real maxValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                return r*2.0;
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxHazardRate;
        }
        // update with new guess
        static void updateGuess(std::vector<Real>& data,
                                Real rate,
                                Size i) {
            data[i] = rate;
            if (i==1)
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

        // guesses
        template <class C>
        static Real guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return detail::avgHazardRate;

            // extrapolate
            Date d = c->dates()[i];
            return c->defaultDensity(d, true);
        }

        // constraints
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::min_element(c->data().begin(), c->data().end()));
                return r/2.0;
            }
            return QL_EPSILON;
        }
        template <class C>
        static Real maxValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                return r*2.0;
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxHazardRate;
        }

        // update with new guess
        static void updateGuess(std::vector<Real>& data,
                                Real density,
                                Size i) {
            data[i] = density;
            if (i==1)
                data[0] = density; // first point is updated as well
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 30; }
    };

}

#endif


#ifndef id_3a4dfae621821fd09c98eae4f1798f21
#define id_3a4dfae621821fd09c98eae4f1798f21
inline bool test_3a4dfae621821fd09c98eae4f1798f21(const int* i) {
    return i != nullptr;
}
#endif
