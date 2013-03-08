/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2011 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon

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

/*! \file bootstraptraits.hpp
    \brief bootstrap traits
*/

#ifndef ql_bootstrap_traits_hpp
#define ql_bootstrap_traits_hpp

#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/forwardcurve.hpp>
#include <ql/termstructures/bootstraphelper.hpp>

namespace QuantLib {

    namespace detail {
        const Real avgRate = 0.05;
        const Real maxRate = 1.0;
    }

    //! Discount-curve traits
    struct Discount {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedDiscountCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<YieldTermStructure> helper;

        // start of curve data
        static Date initialDate(const YieldTermStructure* c) {
            return c->referenceDate();
        }
        // value at reference date
        static Real initialValue(const YieldTermStructure*) {
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
                return 1.0/(1.0+detail::avgRate*c->times()[1]);

            // flat rate extrapolation
            Real r = -std::log(c->data()[i-1])/c->times()[i-1];
            return std::exp(-r * c->times()[i]);
        }

        // possible constraints based on previous values
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                #if defined(QL_NEGATIVE_RATES)
                return *(std::min_element(c->data().begin(),
                                          c->data().end()))/2.0;
                #else
                return c->data().back()/2.0;
                #endif
            }
            Time dt = c->times()[i] - c->times()[i-1];
            return c->data()[i-1] * std::exp(- detail::maxRate * dt);
        }
        template <class C>
        static Real maxValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            #if defined(QL_NEGATIVE_RATES)
            Time dt = c->times()[i] - c->times()[i-1];
            return c->data()[i-1] * std::exp(detail::maxRate * dt);
            #else
            // discounts cannot increase
            return c->data()[i-1];
            #endif
        }

        // root-finding update
        static void updateGuess(std::vector<Real>& data,
                                Real discount,
                                Size i) {
            data[i] = discount;
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 100; }
    };


    //! Zero-curve traits
    struct ZeroYield {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedZeroCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<YieldTermStructure> helper;

        // start of curve data
        static Date initialDate(const YieldTermStructure* c) {
            return c->referenceDate();
        }
        // dummy value at reference date
        static Real initialValue(const YieldTermStructure*) {
            return detail::avgRate;
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
                return detail::avgRate;

            // extrapolate
            Date d = c->dates()[i]; 
            return c->zeroRate(d, c->dayCounter(),
                               Continuous, Annual, true);
        }

        // possible constraints based on previous values
        template <class C>
        static Real minValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::min_element(c->data().begin(), c->data().end()));
                #if defined(QL_NEGATIVE_RATES)
                return r<0.0 ? r*2.0 : r/2.0;
                #else
                return r/2.0;
                #endif
            }
            #if defined(QL_NEGATIVE_RATES)
            // no constraints.
            // We choose as min a value very unlikely to be exceeded.
            return -detail::maxRate;
            #else
            return QL_EPSILON;
            #endif
        }
        template <class C>
        static Real maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                #if defined(QL_NEGATIVE_RATES)
                return r<0.0 ? r/2.0 : r*2.0;
                #else
                return r*2.0;
                #endif
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxRate;
        }

        // root-finding update
        static void updateGuess(std::vector<Real>& data,
                                Real rate,
                                Size i) {
            data[i] = rate;
            if (i==1)
                data[0] = rate; // first point is updated as well
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 100; }
    };


    //! Forward-curve traits
    struct ForwardRate {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedForwardCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<YieldTermStructure> helper;

        // start of curve data
        static Date initialDate(const YieldTermStructure* c) {
            return c->referenceDate();
        }
        // dummy value at reference date
        static Real initialValue(const YieldTermStructure*) {
            return detail::avgRate;
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
                return detail::avgRate;

            // extrapolate
            Date d = c->dates()[i];
            return c->forwardRate(d, d, c->dayCounter(),
                                  Continuous, Annual, true);
        }

        // possible constraints based on previous values
        template <class C>
        static Real minValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::min_element(c->data().begin(), c->data().end()));
                #if defined(QL_NEGATIVE_RATES)
                return r<0.0 ? r*2.0 : r/2.0;
                #else
                return r/2.0;
                #endif
            }
            #if defined(QL_NEGATIVE_RATES)
            // no constraints.
            // We choose as min a value very unlikely to be exceeded.
            return -detail::maxRate;
            #else
            return QL_EPSILON;
            #endif
        }
        template <class C>
        static Real maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                #if defined(QL_NEGATIVE_RATES)
                return r<0.0 ? r/2.0 : r*2.0;
                #else
                return r*2.0;
                #endif
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxRate;
        }

        // root-finding update
        static void updateGuess(std::vector<Real>& data,
                                Real forward,
                                Size i) {
            data[i] = forward;
            if (i==1)
                data[0] = forward; // first point is updated as well
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 100; }
    };

}

#endif
