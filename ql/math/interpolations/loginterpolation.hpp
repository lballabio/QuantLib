/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2008 Ferdinando Ametrano
 Copyright (C) 2004, 2007, 2008 StatPro Italia srl

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

/*! \file loginterpolation.hpp
    \brief log-linear and log-cubic interpolation between discrete points
*/

#ifndef quantlib_log_interpolation_hpp
#define quantlib_log_interpolation_hpp

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/cubicspline.hpp>
#include <ql/math/interpolations/constrainedcubicspline.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2, class Interpolator>
        class LogInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            LogInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                 const I2& yBegin,
                                 const Interpolator& factory = Interpolator())
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              logY_(xEnd-xBegin) {
                interpolation_ = factory.interpolate(this->xBegin_,
                                                     this->xEnd_,
                                                     logY_.begin());
            }
            void update() {
                for (Size i=0; i<logY_.size(); ++i) {
                    QL_REQUIRE(this->yBegin_[i]>0.0,
                               "invalid value (" << this->yBegin_[i]
                               << ") at index " << i);
                    logY_[i] = std::log(this->yBegin_[i]);
                }
                interpolation_.update();
            }
            Real value(Real x) const {
                return std::exp(interpolation_(x, true));
            }
            Real primitive(Real) const {
                QL_FAIL("LogInterpolation primitive not implemented");
            }
            Real derivative(Real x) const {
                return value(x)*interpolation_.derivative(x, true);
            }
            Real secondDerivative(Real x) const {
                return derivative(x)*interpolation_.derivative(x, true) +
                            value(x)*interpolation_.secondDerivative(x, true);
            }
          private:
            std::vector<Real> logY_;
            Interpolation interpolation_;
        };

    }

    //! %log-linear interpolation between discrete points
    class LogLinearInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LogLinearInterpolation(const I1& xBegin, const I1& xEnd,
                               const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                new detail::LogInterpolationImpl<I1,I2,Linear>(
                                                       xBegin, xEnd, yBegin));
            impl_->update();
        }
    };

    //! %log-cubic interpolation between discrete points
    class LogCubicInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LogCubicInterpolation(
                    const I1& xBegin, const I1& xEnd,
                    const I2& yBegin,
                    CubicSplineInterpolation::BoundaryCondition leftCondition,
                    Real leftConditionValue,
                    CubicSplineInterpolation::BoundaryCondition rightCondition,
                    Real rightConditionValue,
                    bool monotonicityConstraint) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                new detail::LogInterpolationImpl<I1,I2,CubicSpline>(
                                        xBegin, xEnd, yBegin,
                                        CubicSpline(leftCondition,
                                                    leftConditionValue,
                                                    rightCondition,
                                                    rightConditionValue,
                                                    monotonicityConstraint)));
            impl_->update();
        }
    };

    //! %log-ConstrainedCubic interpolation between discrete points
    class ConstrainedLogCubicInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        ConstrainedLogCubicInterpolation(
                    const I1& xBegin, const I1& xEnd,
                    const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                new detail::LogInterpolationImpl<I1,I2,ConstrainedCubicSpline>(
                                        xBegin, xEnd, yBegin));
            impl_->update();
        }
    };

    // convenience classes

    //! %LogCubic spline with null second derivative at end points
    class NaturalLogCubic : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        NaturalLogCubic(const I1& xBegin, const I1& xEnd,
                        const I2& yBegin)
        : LogCubicInterpolation(xBegin,xEnd,yBegin,
                      CubicSplineInterpolation::SecondDerivative, 0.0,
                      CubicSplineInterpolation::SecondDerivative, 0.0,
                      false) {}
    };

    //! Natural LogCubic spline with monotonicity constraint
    class MonotonicNaturalLogCubic : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MonotonicNaturalLogCubic(const I1& xBegin, const I1& xEnd,
                                 const I2& yBegin)
        : LogCubicInterpolation(xBegin,xEnd,yBegin,
                      CubicSplineInterpolation::SecondDerivative, 0.0,
                      CubicSplineInterpolation::SecondDerivative, 0.0,
                      true) {}
    };


    //! log-linear interpolation factory and traits
    class LogLinear {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return LogLinearInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = false;
        static const Size requiredPoints = 2;
    };


    //! log-cubic interpolation factory and traits
    class LogCubic {
      public:
        LogCubic(CubicSplineInterpolation::BoundaryCondition leftCondition
                    //= CubicSplineInterpolation::NotAKnot,
                    = CubicSplineInterpolation::SecondDerivative,
                 Real leftConditionValue = 0.0,
                 CubicSplineInterpolation::BoundaryCondition rightCondition
                    = CubicSplineInterpolation::SecondDerivative,
                 Real rightConditionValue = 0.0,
                 bool monotonicityConstraint = true)
        : lefType_(leftCondition), rightType_(rightCondition),
          leftValue_(leftConditionValue), rightValue_(rightConditionValue),
          monotonic_(monotonicityConstraint) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return LogCubicInterpolation(xBegin, xEnd, yBegin,
                                         lefType_,leftValue_,
                                         rightType_, rightValue_,
                                         monotonic_);
        }
        static const bool global = true;
        static const Size requiredPoints = 2;
      private:
        CubicSplineInterpolation::BoundaryCondition lefType_, rightType_;
        Real leftValue_, rightValue_;
        bool monotonic_;
    };

    //! Constrained log-cubic interpolation factory and traits
    class ConstrainedLogCubic {
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return ConstrainedLogCubicInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = true;
        static const Size requiredPoints = 2;
    };

}

#endif
