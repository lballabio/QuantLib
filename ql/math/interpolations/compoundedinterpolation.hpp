/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl

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

/*! \file compoundedinterpolation.hpp
    \brief interpolation over the compounded rate R between discrete points
	R = m(exp(r/m)-1) where r is the original stored value (interpreted as 
	continuously compounded zero rate)
	and m is the compounding frequency
	The structure of the classes here are analogous to those 
	in loginterpolation.hpp
	where the log is replaced by the formula above.
*/

#ifndef quantlib_compounded_interpolation_hpp
#define quantlib_compounded_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {
        template<class I1, class I2, class I> class CompoundedInterpolationImpl;
    }

    //! %Compounded-Linear interpolation between discrete points
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class CompoundedLinearInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CompoundedLinearInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin, Real compFreq) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::CompoundedInterpolationImpl<I1, I2, Linear>(xBegin, xEnd,
                                                       yBegin, compFreq));
            impl_->update();
        }
    };

    //! %CompoundedLinear-interpolation factory and traits
    /*! \ingroup interpolations */
    class CompoundedLinear {
      public:
        explicit CompoundedLinear(Real compFreq)
        : compFreq_(compFreq) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return CompoundedLinearInterpolation(xBegin, xEnd, yBegin, compFreq_);
        }
        static const bool global = false;
        static const Size requiredPoints = 2;
      private:
        Real compFreq_;
    };

    //! %log-cubic interpolation between discrete points
    /*! \ingroup interpolations */
    class CompoundedCubicInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CompoundedCubicInterpolation(const I1& xBegin, const I1& xEnd,
                              const I2& yBegin, Real compFreq,
                              CubicInterpolation::DerivativeApprox da,
                              bool monotonic,
                              CubicInterpolation::BoundaryCondition leftC,
                              Real leftConditionValue,
                              CubicInterpolation::BoundaryCondition rightC,
                              Real rightConditionValue) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::CompoundedInterpolationImpl<I1, I2, Cubic>(
                    xBegin, xEnd, yBegin, compFreq,
                    Cubic(da, monotonic,
                          leftC, leftConditionValue,
                          rightC, rightConditionValue)));
            impl_->update();
        }
    };

    //! log-cubic interpolation factory and traits
    /*! \ingroup interpolations */
    class CompoundedCubic {
      public:
        CompoundedCubic(Real compFreq, CubicInterpolation::DerivativeApprox da,
                  bool monotonic = true,
                  CubicInterpolation::BoundaryCondition leftCondition
                      = CubicInterpolation::SecondDerivative,
                  Real leftConditionValue = 0.0,
                  CubicInterpolation::BoundaryCondition rightCondition
                      = CubicInterpolation::SecondDerivative,
                  Real rightConditionValue = 0.0)
        : compFreq_(compFreq), da_(da), monotonic_(monotonic),
          leftType_(leftCondition), rightType_(rightCondition),
          leftValue_(leftConditionValue), rightValue_(rightConditionValue) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return CompoundedCubicInterpolation(xBegin, xEnd, yBegin, compFreq_,
                                         da_, monotonic_,
                                         leftType_, leftValue_,
                                         rightType_, rightValue_);
        }
        static const bool global = true;
        static const Size requiredPoints = 2;
      private:
        Real compFreq_;
        CubicInterpolation::DerivativeApprox da_;
        bool monotonic_;
        CubicInterpolation::BoundaryCondition leftType_, rightType_;
        Real leftValue_, rightValue_;
    };

    namespace detail {

        template <class I1, class I2, class Interpolator>
        class CompoundedInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            CompoundedInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin, Real compFreq,
									const Interpolator& factory = Interpolator())
            : compFreq_(compFreq), Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                 Interpolator::requiredPoints),
              compY_(xEnd-xBegin) {
                QL_REQUIRE(compFreq_ > 0,
                            "Compounding frequency must be positive"); 
                interpolation_ = factory.interpolate(this->xBegin_,
                                                     this->xEnd_,
                                                     compY_.begin());
            }
            void update() {
                for (Size i=0; i<compY_.size(); ++i) {
					//Formula: y = m[exp(x/m)-1], where x = continuous compounded zero rate
					//and y = discretely compounded zero rate with compounding frequency m
                    compY_[i] = compFreq_*(std::exp(this->yBegin_[i]/compFreq_) - 1);
                }
                interpolation_.update();
            }
            Real value(Real x) const {
				//Formula: x = m*ln(1+y/m)
                return compFreq_*std::log(1.0+interpolation_(x, true)/compFreq_);
            }
            Real primitive(Real) const {
                QL_FAIL("CompoundedInterpolation primitive not implemented");
            }
            Real derivative(Real x) const {
				//Formula: x' = m*y'/(m+y)
				Real denom = compFreq_ + interpolation_(x, true);
                QL_REQUIRE(denom != 0.0,
                            "Derivative cannot be calculated for given rate"); 
                return compFreq_*interpolation_.derivative(x, true)/denom;
            }
            Real secondDerivative(Real x) const {
				//Formula: x'' = m[y''(m+y)-y'^2]/[(m+y)^2]
				Real sum = compFreq_ + interpolation_(x, true);
				Real denom = sum*sum;
                QL_REQUIRE(denom != 0.0,
                            "Derivative cannot be calculated for given rate");
				Real deriv = interpolation_.derivative(x, true);
				Real derivSq = deriv*deriv;
				Real deriv2 = interpolation_.secondDerivative(x, true);
                return compFreq_*(deriv2*sum-derivSq)/denom;
            }
          private:
            std::vector<Real> compY_;//analogous to compY_ in LogInterpolationImpl
            Interpolation interpolation_;
			Real compFreq_;
        };

    }

}

#endif
