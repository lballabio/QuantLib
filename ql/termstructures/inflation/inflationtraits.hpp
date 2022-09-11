/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2007, 2008 StatPro Italia srl
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file inflationtraits.hpp
    \brief inflation bootstrap traits
*/

#ifndef ql_inflation_bootstrap_traits_hpp
#define ql_inflation_bootstrap_traits_hpp

#include <ql/termstructures/inflation/interpolatedzeroinflationcurve.hpp>
#include <ql/termstructures/inflation/interpolatedyoyinflationcurve.hpp>
#include <ql/termstructures/bootstraphelper.hpp>

namespace QuantLib {

    namespace detail {
        constexpr Rate avgInflation = 0.02;
        constexpr Rate maxInflation = 0.5;
    }

    //! Bootstrap traits to use for PiecewiseZeroInflationCurve
    class ZeroInflationTraits {
      public:
        typedef BootstrapHelper<ZeroInflationTermStructure> helper;

        // start of curve data
        static Date initialDate(const ZeroInflationTermStructure* t) {
            return inflationPeriod(t->referenceDate() - t->observationLag(), t->frequency()).first;
        }
        // value at reference date
        static Rate initialValue(const ZeroInflationTermStructure* t) {
            return t->baseRate();
        }

        // guesses
        template <class C>
        static Rate guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return detail::avgInflation;

            // could/should extrapolate
            return detail::avgInflation;
        }

        // constraints
        template <class C>
        static Rate minValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Rate r = *(std::min_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r*2.0) : r/2.0;
            }
            return -detail::maxInflation;
        }
        template <class C>
        static Rate maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Rate r = *(std::max_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r/2.0) : r*2.0;
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxInflation;
        }

        // update with new guess
        static void updateGuess(std::vector<Rate>& data,
                                Rate level,
                                Size i) {
            data[i] = level;
        }
        // upper bound for convergence loop
        // calibration is trivial, should be immediate
        static Size maxIterations() { return 5; }
    };

    //! Bootstrap traits to use for PiecewiseZeroInflationCurve
    class YoYInflationTraits {
      public:
        // helper class
        typedef BootstrapHelper<YoYInflationTermStructure> helper;

        // start of curve data
        static Date initialDate(const YoYInflationTermStructure* t) {
            if (t->indexIsInterpolated()) {
                return t->referenceDate() - t->observationLag();
            } else {
                return inflationPeriod(t->referenceDate() - t->observationLag(),
                                       t->frequency()).first;
            }
        }
        // value at reference date
        static Rate initialValue(const YoYInflationTermStructure* t) {
            return t->baseRate();
        }

        // guesses
        template <class C>
        static Rate guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return detail::avgInflation;
        
            // could/should extrapolate
            return detail::avgInflation;
        }

        // constraints
        template <class C>
        static Rate minValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Rate r = *(std::min_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r*2.0) : r/2.0;
            }
            return -detail::maxInflation;
        }
        template <class C>
        static Rate maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Rate r = *(std::max_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r/2.0) : r*2.0;
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxInflation;
        }

        // update with new guess
        static void updateGuess(std::vector<Rate>& data,
                                Rate level,
                                Size i) {
            data[i] = level;
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 40; }
    };

}

#endif
