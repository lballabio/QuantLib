/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2008, 2009 Ferdinando Ametrano
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
#include <ql/math/interpolations/cubicinterpolation.hpp>
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
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::LogInterpolationImpl<I1, I2, Linear>(xBegin, xEnd,
                                                             yBegin));
            impl_->update();
        }
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

    //! %log-cubic interpolation between discrete points
    class LogCubicInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LogCubicInterpolation(const I1& xBegin, const I1& xEnd,
                              const I2& yBegin,
                              CubicInterpolation::DerivativeApprox da,
                              bool monotonic,
                              CubicInterpolation::BoundaryCondition leftC,
                              Real leftConditionValue,
                              CubicInterpolation::BoundaryCondition rightC,
                              Real rightConditionValue) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::LogInterpolationImpl<I1, I2, Cubic>(
                    xBegin, xEnd, yBegin,
                    Cubic(da, monotonic,
                          leftC, leftConditionValue,
                          rightC, rightConditionValue)));
            impl_->update();
        }
    };

    //! log-cubic interpolation factory and traits
    class LogCubic {
      public:
        LogCubic(CubicInterpolation::DerivativeApprox da,
                  bool monotonic = true,
                  CubicInterpolation::BoundaryCondition leftCondition
                      = CubicInterpolation::SecondDerivative,
                  Real leftConditionValue = 0.0,
                  CubicInterpolation::BoundaryCondition rightCondition
                      = CubicInterpolation::SecondDerivative,
                  Real rightConditionValue = 0.0)
        : da_(da), monotonic_(monotonic),
          leftType_(leftCondition), rightType_(rightCondition),
          leftValue_(leftConditionValue), rightValue_(rightConditionValue) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return LogCubicInterpolation(xBegin, xEnd, yBegin,
                                         da_, monotonic_,
                                         leftType_, leftValue_,
                                         rightType_, rightValue_);
        }
        static const bool global = true;
        static const Size requiredPoints = 2;
      private:
        CubicInterpolation::DerivativeApprox da_;
        bool monotonic_;
        CubicInterpolation::BoundaryCondition leftType_, rightType_;
        Real leftValue_, rightValue_;
    };

    // convenience classes

    class LogCubicNaturalSpline : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LogCubicNaturalSpline(const I1& xBegin,
                              const I1& xEnd,
                              const I2& yBegin)
        : LogCubicInterpolation(xBegin, xEnd, yBegin,
                                CubicInterpolation::Spline, false,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MonotonicLogCubicNaturalSpline : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MonotonicLogCubicNaturalSpline(const I1& xBegin,
                                       const I1& xEnd,
                                       const I2& yBegin)
        : LogCubicInterpolation(xBegin, xEnd, yBegin,
                                CubicInterpolation::Spline, true,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class KrugerLogCubic : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        KrugerLogCubic(const I1& xBegin,
                       const I1& xEnd,
                       const I2& yBegin)
        : LogCubicInterpolation(xBegin, xEnd, yBegin,
                                CubicInterpolation::Kruger, false,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class FritschButlandLogCubic : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        FritschButlandLogCubic(const I1& xBegin,
                               const I1& xEnd,
                               const I2& yBegin)
        : LogCubicInterpolation(xBegin, xEnd, yBegin,
                                CubicInterpolation::FritschButland, false,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class LogParabolic : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LogParabolic(const I1& xBegin,
                     const I1& xEnd,
                     const I2& yBegin)
        : LogCubicInterpolation(xBegin, xEnd, yBegin,
                                CubicInterpolation::Parabolic, false,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MonotonicLogParabolic : public LogCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MonotonicLogParabolic(const I1& xBegin,
                              const I1& xEnd,
                              const I2& yBegin)
        : LogCubicInterpolation(xBegin, xEnd, yBegin,
                                CubicInterpolation::Parabolic, true,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0) {}
    };

}

#endif
