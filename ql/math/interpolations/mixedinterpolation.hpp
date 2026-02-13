/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Ferdinando Ametrano

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

        template <class I1, class I2>
        class MixedInterpolationImpl;

    }


    struct MixedInterpolation {
        enum Behavior {
            ShareRanges,  /*!< Define both interpolations over the
                               whole range defined by the passed
                               iterators. This is the default
                               behavior. */
            SplitRanges   /*!< Define the first interpolation over the
                               first part of the range, and the second
                               interpolation over the second part. */
        };
    };

    //! mixed linear/cubic interpolation between discrete points
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class MixedLinearCubicInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearCubicInterpolation(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin, Size n,
                                      MixedInterpolation::Behavior behavior,
                                      CubicInterpolation::DerivativeApprox da,
                                      bool monotonic,
                                      CubicInterpolation::BoundaryCondition leftC,
                                      Real leftConditionValue,
                                      CubicInterpolation::BoundaryCondition rightC,
                                      Real rightConditionValue) {
            impl_ = ext::make_shared<detail::MixedInterpolationImpl<I1, I2>>(
                xBegin, xEnd, yBegin, n, behavior,
                Linear(),
                Cubic(da, monotonic,
                      leftC, leftConditionValue,
                      rightC, rightConditionValue));
            impl_->update();
        }
    };

    //! mixed linear/cubic interpolation factory and traits
    /*! \ingroup interpolations */
    class MixedLinearCubic {
      public:
        MixedLinearCubic(Size n,
                         MixedInterpolation::Behavior behavior,
                         CubicInterpolation::DerivativeApprox da,
                         bool monotonic = true,
                         CubicInterpolation::BoundaryCondition leftCondition
                             = CubicInterpolation::SecondDerivative,
                         Real leftConditionValue = 0.0,
                         CubicInterpolation::BoundaryCondition rightCondition
                             = CubicInterpolation::SecondDerivative,
                         Real rightConditionValue = 0.0)
        : n_(n), behavior_(behavior), da_(da), monotonic_(monotonic),
          leftType_(leftCondition), rightType_(rightCondition),
          leftValue_(leftConditionValue), rightValue_(rightConditionValue) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return MixedLinearCubicInterpolation(xBegin, xEnd,
                                                 yBegin, n_, behavior_,
                                                 da_, monotonic_,
                                                 leftType_, leftValue_,
                                                 rightType_, rightValue_);
        }
        // fix below
        static const bool global = true;
        static const Size requiredPoints = 3;
      private:
        Size n_;
        MixedInterpolation::Behavior behavior_;
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
                                      const I2& yBegin, Size n,
                                      MixedInterpolation::Behavior behavior
                                            = MixedInterpolation::ShareRanges)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n, behavior,
                                        CubicInterpolation::Spline, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearMonotonicCubicNaturalSpline : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearMonotonicCubicNaturalSpline(const I1& xBegin, const I1& xEnd,
                                               const I2& yBegin, Size n,
                                               MixedInterpolation::Behavior behavior
                                                   = MixedInterpolation::ShareRanges)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n, behavior,
                                        CubicInterpolation::Spline, true,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearKrugerCubic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearKrugerCubic(const I1& xBegin, const I1& xEnd,
                               const I2& yBegin, Size n,
                               MixedInterpolation::Behavior behavior
                                            = MixedInterpolation::ShareRanges)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n, behavior,
                                        CubicInterpolation::Kruger, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearFritschButlandCubic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearFritschButlandCubic(const I1& xBegin, const I1& xEnd,
                                       const I2& yBegin, Size n,
                                       MixedInterpolation::Behavior behavior
                                            = MixedInterpolation::ShareRanges)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n, behavior,
                                        CubicInterpolation::FritschButland, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearParabolic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearParabolic(const I1& xBegin, const I1& xEnd,
                             const I2& yBegin, Size n,
                             MixedInterpolation::Behavior behavior
                                            = MixedInterpolation::ShareRanges)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n, behavior,
                                        CubicInterpolation::Parabolic, false,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    class MixedLinearMonotonicParabolic : public MixedLinearCubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MixedLinearMonotonicParabolic(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin, Size n,
                                      MixedInterpolation::Behavior behavior
                                            = MixedInterpolation::ShareRanges)
        : MixedLinearCubicInterpolation(xBegin, xEnd, yBegin, n, behavior,
                                        CubicInterpolation::Parabolic, true,
                                        CubicInterpolation::SecondDerivative, 0.0,
                                        CubicInterpolation::SecondDerivative, 0.0) {}
    };

    namespace detail {

        template <class I1, class I2>
        class MixedInterpolationImpl final
            : public Interpolation::templateImpl<I1, I2> {
          public:
            template <class Interpolator1, class Interpolator2>
            MixedInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                   const I2& yBegin, Size n,
                                   MixedInterpolation::Behavior behavior,
                                   const Interpolator1& factory1,
                                   const Interpolator2& factory2)
            : Interpolation::templateImpl<I1, I2>(xBegin, xEnd, yBegin, 1) {
                Size maxN = static_cast<Size>(xEnd - xBegin);
                // SplitRanges needs xBegin2_+1 to be valid
                if (behavior == MixedInterpolation::SplitRanges) {
                    --maxN;
                }
                // This only checks that we pass valid iterators into interpolate()
                // calls below. The calls themselves check requiredPoints for each
                // of the segments.
                QL_REQUIRE(n <= maxN, "n is too large (" << n << " > " << maxN << ")");

                xBegin2_ = this->xBegin_ + n;

                switch (behavior) {
                  case MixedInterpolation::ShareRanges:
                    interpolation1_ = factory1.interpolate(this->xBegin_,
                                                           this->xEnd_,
                                                           this->yBegin_);
                    interpolation2_ = factory2.interpolate(this->xBegin_,
                                                           this->xEnd_,
                                                           this->yBegin_);
                    break;
                  case MixedInterpolation::SplitRanges:
                    interpolation1_ = factory1.interpolate(this->xBegin_,
                                                           this->xBegin2_ + 1,
                                                           this->yBegin_);
                    interpolation2_ = factory2.interpolate(this->xBegin2_,
                                                           this->xEnd_,
                                                           this->yBegin_ + n);
                    break;
                  default:
                    QL_FAIL("unknown mixed-interpolation behavior: " << behavior);
                }
            }

            void update() {
                interpolation1_.update();
                interpolation2_.update();
            }
            Real value(Real x) const {
                if (x<*xBegin2_)
                    return interpolation1_(x, true);
                return interpolation2_(x, true);
            }
            Real primitive(Real x) const {
                if (x<*xBegin2_)
                    return interpolation1_.primitive(x, true);
                return interpolation2_.primitive(x, true) -
                    interpolation2_.primitive(*xBegin2_, true) +
                    interpolation1_.primitive(*xBegin2_, true);
            }
            Real derivative(Real x) const {
                if (x<*xBegin2_)
                    return interpolation1_.derivative(x, true);
                return interpolation2_.derivative(x, true);
            }
            Real secondDerivative(Real x) const {
                if (x<*xBegin2_)
                    return interpolation1_.secondDerivative(x, true);
                return interpolation2_.secondDerivative(x, true);
            }
          private:
            I1 xBegin2_;
            Interpolation interpolation1_, interpolation2_;
        };

    }

}

#endif
