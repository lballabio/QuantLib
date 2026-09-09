/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2011 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2019 SoftSolutions! S.r.l.
 Copyright (C) 2026 Kyrylo Protsenko

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

/*! \file bootstraptraits.hpp
    \brief bootstrap traits
*/

#ifndef ql_bootstrap_traits_hpp
#define ql_bootstrap_traits_hpp

#include <ql/math/array.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/interpolatedsimplezerocurve.hpp>
#include <ql/termstructures/yield/forwardcurve.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <algorithm>
#include <utility>

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

        // guess for the whole curve and whether it needs to be transformed
        template <class C>
        static std::pair<Array, bool> globalGuess(const C* c, bool validData)
        {
            if (validData)
                return {Array(c->data().begin() + 1, c->data().end()), true};

            Array guess(c->times().begin() + 1, c->times().end());
            guess *= -detail::avgRate;
            // Discount::transformInverse() is log(), so we simply don't call exp() here
            // and return false for needsTransform. This saves one pair of exp()/log()
            // calls per pillar.
            return {std::move(guess), false};
        }

        // possible constraints based on previous values
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                return *(std::min_element(c->data().begin(),
                                          c->data().end()))/2.0;
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
            Time dt = c->times()[i] - c->times()[i-1];
            return c->data()[i-1] * std::exp(detail::maxRate * dt);
        }

        // transformation to add constraints to an unconstrained optimization
        template <class C>
        static Real transformDirect(Real x, Size i, const C* c)
        {
            return std::exp(x);
        }
        template <class C>
        static Real transformInverse(Real x, Size i, const C* c)
        {
            return std::log(x);
        }

        // root-finding update
        static void updateGuess(std::vector<Real>& data,
                                Real discount,
                                Size i) {
            data[i] = discount;
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 100; }

        static constexpr bool supportsAnalyticJacobian = true;
        template <class C>
        static Real discountFactorDerivative(Time, const C*) {
            // curve data are discount factors
            return 1.0;
        }
        template <class C>
        static std::vector<std::pair<Size, Real>> extrapolationNodeWeights(
            Time t, const C* c,
                                 const Interpolation& interpolation) {
            // InterpolatedDiscountCurve extrapolates with the instantaneous
            // forward at its last node. Its tail therefore depends on both
            // the endpoint discount and the endpoint interpolation slope.
            Time tMax = c->times().back();
            DiscountFactor dMax = c->data().back();
            Real slope = interpolation.derivative(tMax, true);
            auto valueWeights = interpolation.nodeWeights(tMax, true);
            auto slopeWeights =
                interpolation.derivativeNodeWeights(tMax, true);
            if (valueWeights.empty() || slopeWeights.empty())
                return {};

            std::vector<std::pair<Size, Real>> result;
            result.reserve(valueWeights.size() + slopeWeights.size());
            DiscountFactor d = c->discount(t, true);
            Time dt = t-tMax;
            for (const auto& [j, w] : valueWeights) {
                Real coefficient = d*w*(1.0/dMax-dt*slope/(dMax*dMax));
                result.emplace_back(j, coefficient);
            }
            for (const auto& [j, w] : slopeWeights) {
                Real coefficient = d*dt*w/dMax;
                auto found = std::find_if(
                    result.begin(), result.end(),
                    [j = j](const auto& entry) { return entry.first == j; });
                if (found == result.end())
                    result.emplace_back(j, coefficient);
                else
                    found->second += coefficient;
            }
            return result;
        }
        // updateGuess leaves data[0] fixed
        static constexpr bool firstDataPointTracksSecond = false;
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

        // guess for the whole curve and whether it needs to be transformed
        template <class C>
        static std::pair<Array, bool> globalGuess(const C* c, bool validData)
        {
            if (validData)
                return {Array(c->data().begin() + 1, c->data().end()), true};

            return {Array(c->times().size() - 1, detail::avgRate), true};
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
                return r<0.0 ? Real(r*2.0) : Real(r/2.0);
            }
            // no constraints.
            // We choose as min a value very unlikely to be exceeded.
            return -detail::maxRate;
        }
        template <class C>
        static Real maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r/2.0) : Real(r*2.0);
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

        static constexpr bool supportsAnalyticJacobian = true;
        template <class C>
        static Real discountFactorDerivative(Time t, const C* c) {
            // P(t) = exp(-z(t)*t) with interpolated z
            return -t * c->discount(t, true);
        }
        template <class C>
        static std::vector<std::pair<Size, Real>> extrapolationNodeWeights(
            Time t, const C* c,
                                 const Interpolation& interpolation) {
            Time tMax = c->times().back();
            auto valueWeights = interpolation.nodeWeights(tMax, true);
            auto slopeWeights =
                interpolation.derivativeNodeWeights(tMax, true);
            if (valueWeights.empty() || slopeWeights.empty())
                return {};

            Real slopeScale = tMax*(t-tMax)/t;
            for (const auto& [j, w] : slopeWeights) {
                auto found = std::find_if(
                    valueWeights.begin(), valueWeights.end(),
                    [j = j](const auto& entry) { return entry.first == j; });
                if (found == valueWeights.end())
                    valueWeights.emplace_back(j, slopeScale*w);
                else
                    found->second += slopeScale*w;
            }
            return valueWeights;
        }
        static constexpr bool firstDataPointTracksSecond = true;
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

        // guess for the whole curve and whether it needs to be transformed
        template <class C>
        static std::pair<Array, bool> globalGuess(const C* c, bool validData)
        {
            if (validData)
                return {Array(c->data().begin() + 1, c->data().end()), true};

            return {Array(c->times().size() - 1, detail::avgRate), true};
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
                return r<0.0 ? Real(r*2.0) : Real(r/2.0);
            }
            // no constraints.
            // We choose as min a value very unlikely to be exceeded.
            return -detail::maxRate;
        }
        template <class C>
        static Real maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r/2.0) : Real(r*2.0);
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

    //! Simple Zero-curve traits
    struct SimpleZeroYield {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedSimpleZeroCurve<Interpolator> type;
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
                               Simple, Annual, true);
        }

        // guess for the whole curve and whether it needs to be transformed
        template <class C>
        static std::pair<Array, bool> globalGuess(const C* c, bool validData)
        {
            if (validData)
                return {Array(c->data().begin() + 1, c->data().end()), true};

            return {Array(c->times().size() - 1, detail::avgRate), true};
        }

        // possible constraints based on previous values
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            Real result;
            if (validData) {
                Real r = *(std::min_element(c->data().begin(), c->data().end()));
                result = r<0.0 ? Real(r*2.0) : r/2.0;
            } else {
                // no constraints.
                // We choose as min a value very unlikely to be exceeded.
                result = -detail::maxRate;
            }
            return std::max(result, -1.0 / c->times()[i] + 1E-8);
        }
        template <class C>
        static Real maxValueAfter(Size,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(), c->data().end()));
                return r<0.0 ? Real(r/2.0) : r*2.0;
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxRate;
        }

        // transformation to add constraints to an unconstrained optimization
        template <class C>
        static Real transformDirect(Real x, Size i, const C* c)
        {
            return std::exp(x) + (-1.0 / c->times()[i] + 1E-8);
        }
        template <class C>
        static Real transformInverse(Real x, Size i, const C* c)
        {
            return std::log(x - (-1.0 / c->times()[i] + 1E-8));
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

        static constexpr bool supportsAnalyticJacobian = true;
        template <class C>
        static Real discountFactorDerivative(Time t, const C* c) {
            // P(t) = 1/(1+z(t)*t) with interpolated z
            DiscountFactor d = c->discount(t, true);
            return -t * d * d;
        }
        template <class C>
        static std::vector<std::pair<Size, Real>> extrapolationNodeWeights(
            Time t, const C* c,
                                 const Interpolation& interpolation) {
            Time tMax = c->times().back();
            auto valueWeights = interpolation.nodeWeights(tMax, true);
            auto slopeWeights =
                interpolation.derivativeNodeWeights(tMax, true);
            if (valueWeights.empty() || slopeWeights.empty())
                return {};

            Real slopeScale = tMax*(t-tMax)/t;
            for (const auto& [j, w] : slopeWeights) {
                auto found = std::find_if(
                    valueWeights.begin(), valueWeights.end(),
                    [j = j](const auto& entry) { return entry.first == j; });
                if (found == valueWeights.end())
                    valueWeights.emplace_back(j, slopeScale*w);
                else
                    found->second += slopeScale*w;
            }
            return valueWeights;
        }
        static constexpr bool firstDataPointTracksSecond = true;
    };


}

#endif
