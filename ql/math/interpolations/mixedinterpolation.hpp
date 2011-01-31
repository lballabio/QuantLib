/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Ferdinando Ametrano

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

/*! \file mixedinterpolation.hpp
    \brief mixed interpolation between discrete points
*/

#ifndef quantlib_mixed_interpolation_hpp
#define quantlib_mixed_interpolation_hpp

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2, class Interpolator1, class Interpolator2>
        class MixedInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            MixedInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                   const I2& yBegin, Size n,
                                   const Interpolator1& factory1 = Interpolator1(),
                                   const Interpolator2& factory2 = Interpolator2())
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              n_(n) {

                xBegin2_ = this->xBegin_+n;

                QL_REQUIRE(xBegin2_<this->xEnd_,
                           "too large n (" << n << ") for " <<
                           this->xEnd_-this->xBegin_ << "-element x sequence");

                interpolation1_ = factory1.interpolate(this->xBegin_,
                                                       this->xEnd_,
                                                       this->yBegin_);
                interpolation2_ = factory2.interpolate(this->xBegin_,
                                                       this->xEnd_,
                                                       this->yBegin_);
            }
            void update() {
                interpolation1_.update();
                interpolation2_.update();
            }
            Real value(Real x) const {
                if (x<*(this->xBegin2_))
                    return interpolation1_(x, true);
                return interpolation2_(x, true);
            }
            Real primitive(Real x) const {
                if (x<*(this->xBegin2_))
                    return interpolation1_.primitive(x, true);
                return interpolation2_.primitive(x, true);
            }
            Real derivative(Real x) const {
                if (x<*(this->xBegin2_))
                    return interpolation1_.derivative(x, true);
                return interpolation2_.derivative(x, true);
            }
            Real secondDerivative(Real x) const {
                if (x<*(this->xBegin2_))
                    return interpolation1_.secondDerivative(x, true);
                return interpolation2_.secondDerivative(x, true);
            }
            Size switchIndex() { return n_; }
          private:
            I1 xBegin2_;
            Size n_;
            Interpolation interpolation1_, interpolation2_;
        };

    }

    //! mixed linear/cubic interpolation between discrete points
    class MixedLinearCubicInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearCubicInterpolation(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin, Size n,
                                      CubicInterpolation::DerivativeApprox da,
                                      bool monotonic,
                                      CubicInterpolation::BoundaryCondition leftC,
                                      Real leftConditionValue,
                                      CubicInterpolation::BoundaryCondition rightC,
                                      Real rightConditionValue) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::MixedInterpolationImpl<I1, I2, Linear, Cubic>(
                    xBegin, xEnd, yBegin, n,
                    Linear(),
                    Cubic(da, monotonic,
                          leftC, leftConditionValue,
                          rightC, rightConditionValue)));
            impl_->update();
        }
        //Size switchIndex() { return impl_->switchIndex(); }
    };

    //! mixed linear/cubic interpolation factory and traits
    class MixedLinearCubic {
      public:
        MixedLinearCubic(Size n,
                         CubicInterpolation::DerivativeApprox da,
                         bool monotonic = true,
                         CubicInterpolation::BoundaryCondition leftCondition
                             = CubicInterpolation::SecondDerivative,
                         Real leftConditionValue = 0.0,
                         CubicInterpolation::BoundaryCondition rightCondition
                             = CubicInterpolation::SecondDerivative,
                         Real rightConditionValue = 0.0)
        : n_(n), da_(da), monotonic_(monotonic),
          leftType_(leftCondition), rightType_(rightCondition),
          leftValue_(leftConditionValue), rightValue_(rightConditionValue) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return MixedLinearCubicInterpolation(xBegin, xEnd,
                                                 yBegin, n_,
                                                 da_, monotonic_,
                                                 leftType_, leftValue_,
                                                 rightType_, rightValue_);
        }
        // fix below
        static const bool global = true;
        static const Size requiredPoints = 3;
      private:
        Size n_;
        CubicInterpolation::DerivativeApprox da_;
        bool monotonic_;
        CubicInterpolation::BoundaryCondition leftType_, rightType_;
        Real leftValue_, rightValue_;
    };

    // convenience classes

    class MixedLinearCubicNaturalSpline : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearCubicNaturalSpline(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin, Size n)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n,
                                        CubicInterpolation::Spline, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearMonotonicCubicNaturalSpline : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearMonotonicCubicNaturalSpline(const I1& xBegin, const I1& xEnd,
                                               const I2& yBegin, Size n)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n,
                                        CubicInterpolation::Spline, true,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearKrugerCubic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearKrugerCubic(const I1& xBegin, const I1& xEnd,
                               const I2& yBegin, Size n)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n,
                                        CubicInterpolation::Kruger, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearFritschButlandCubic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearFritschButlandCubic(const I1& xBegin, const I1& xEnd,
                                       const I2& yBegin, Size n)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n,
                                        CubicInterpolation::FritschButland, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearParabolic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearParabolic(const I1& xBegin, const I1& xEnd,
                             const I2& yBegin, Size n)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n,
                                        CubicInterpolation::Parabolic, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearMonotonicParabolic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearMonotonicParabolic(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin, Size n)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n,
                                        CubicInterpolation::Parabolic, true,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

}

#endif
