/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file bicubicsplineinterpolation.hpp
    \brief bicubic spline interpolation between discrete points
*/

#ifndef quantlib_bicubic_spline_interpolation_hpp
#define quantlib_bicubic_spline_interpolation_hpp

#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>

namespace QuantLib {

    namespace detail {

        class BicubicSplineDerivatives {
          public:
            virtual ~BicubicSplineDerivatives() = default;
            virtual Real derivativeX(Real x, Real y) const = 0;
            virtual Real derivativeY(Real x, Real y) const = 0;
            virtual Real derivativeXY(Real x, Real y) const = 0;
            virtual Real secondDerivativeX(Real x, Real y) const = 0;
            virtual Real secondDerivativeY(Real x, Real y) const = 0;
        };
    
        template <class I1, class I2, class M>
        class BicubicSplineImpl
            : public Interpolation2D::templateImpl<I1,I2,M>,
              public BicubicSplineDerivatives {
          public:
            BicubicSplineImpl(const I1& xBegin, const I1& xEnd,
                              const I2& yBegin, const I2& yEnd,
                              const M& zData)
            : Interpolation2D::templateImpl<I1,I2,M>(xBegin,xEnd,
                                                     yBegin,yEnd,
                                                     zData) {
                calculate();
            }
            void calculate() override {
                splines_.resize(this->zData_.rows());
                for (Size i=0; i<(this->zData_.rows()); ++i)
                    splines_[i] = CubicInterpolation(
                                this->xBegin_, this->xEnd_,
                                this->zData_.row_begin(i),
                                CubicInterpolation::Spline, false,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0);
            }
            Real value(Real x, Real y) const override {
                std::vector<Real> section(splines_.size());
                for (Size i=0; i<splines_.size(); i++)
                    section[i]=splines_[i](x,true);

                CubicInterpolation spline(this->yBegin_, this->yEnd_,
                                          section.begin(),
                                          CubicInterpolation::Spline, false,
                                          CubicInterpolation::SecondDerivative, 0.0,
                                          CubicInterpolation::SecondDerivative, 0.0);
                return spline(y,true);
            }

            Real derivativeX(Real x, Real y) const override {
                std::vector<Real> section(this->zData_.columns());
                for (Size i=0; i < section.size(); ++i) {
                    section[i] = value(this->xBegin_[i], y);
                }
                
                return CubicInterpolation(
                    this->xBegin_, this->xEnd_,
                    section.begin(),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::SecondDerivative, 0.0,
                    CubicInterpolation::SecondDerivative, 0.0).derivative(x);
            }

            Real secondDerivativeX(Real x, Real y) const override {
                std::vector<Real> section(this->zData_.columns());
                for (Size i=0; i < section.size(); ++i) {
                    section[i] = value(this->xBegin_[i], y);
                }
                
                return CubicInterpolation(
                    this->xBegin_, this->xEnd_,
                    section.begin(),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::SecondDerivative, 0.0,
                    CubicInterpolation::SecondDerivative, 0.0)
                                                          .secondDerivative(x);
            }

            Real derivativeY(Real x, Real y) const override {
                std::vector<Real> section(splines_.size());
                for (Size i=0; i<splines_.size(); i++)
                    section[i]=splines_[i](x,true);

                return CubicInterpolation(
                    this->yBegin_, this->yEnd_,
                    section.begin(),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::SecondDerivative, 0.0,
                    CubicInterpolation::SecondDerivative, 0.0).derivative(y);
            }

            Real secondDerivativeY(Real x, Real y) const override {
                std::vector<Real> section(splines_.size());
                for (Size i=0; i<splines_.size(); i++)
                    section[i]=splines_[i](x,true);

                return CubicInterpolation(
                    this->yBegin_, this->yEnd_,
                    section.begin(),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::SecondDerivative, 0.0,
                    CubicInterpolation::SecondDerivative, 0.0)
                                                        .secondDerivative(y);
            }

            Real derivativeXY(Real x, Real y) const override {
                std::vector<Real> section(this->zData_.columns());
                for (Size i=0; i < section.size(); ++i) {
                    section[i] = derivativeY(this->xBegin_[i], y);
                }
                
                return CubicInterpolation(
                    this->xBegin_, this->xEnd_,
                    section.begin(),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::SecondDerivative, 0.0,
                    CubicInterpolation::SecondDerivative, 0.0).derivative(x);
            }

          private:
            std::vector<Interpolation> splines_;
        };

    }

    //! bicubic-spline interpolation between discrete points
    /*! \ingroup interpolations
        \todo revise end conditions
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class BicubicSpline : public Interpolation2D {
      public:
        /*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
        template <class I1, class I2, class M>
        BicubicSpline(const I1& xBegin, const I1& xEnd,
                      const I2& yBegin, const I2& yEnd,
                      const M& zData) {
            impl_ = ext::shared_ptr<Interpolation2D::Impl>(
                  new detail::BicubicSplineImpl<I1,I2,M>(xBegin, xEnd,
                                                         yBegin, yEnd, zData));
        }
        
        Real derivativeX(Real x, Real y) const {
            return ext::dynamic_pointer_cast<detail::BicubicSplineDerivatives>
                    (impl_)->derivativeX(x, y);
        }
        Real derivativeY(Real x, Real y) const {
            return ext::dynamic_pointer_cast<detail::BicubicSplineDerivatives>
                    (impl_)->derivativeY(x, y);
        }
        Real secondDerivativeX(Real x, Real y) const {
            return ext::dynamic_pointer_cast<detail::BicubicSplineDerivatives>
                    (impl_)->secondDerivativeX(x, y);
        }
        Real secondDerivativeY(Real x, Real y) const {
            return ext::dynamic_pointer_cast<detail::BicubicSplineDerivatives>
                    (impl_)->secondDerivativeY(x, y);
        }
        
        Real derivativeXY(Real x, Real y) const {
            return ext::dynamic_pointer_cast<detail::BicubicSplineDerivatives>
                    (impl_)->derivativeXY(x, y);            
        }
    };

    //! bicubic-spline-interpolation factory
    class Bicubic {
      public:
        template <class I1, class I2, class M>
        Interpolation2D interpolate(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin, const I2& yEnd,
                                    const M& z) const {
            return BicubicSpline(xBegin,xEnd,yBegin,yEnd,z);
        }
    };

}

#endif


#ifndef id_041561964e45ff02ffb58f48d7f11c58
#define id_041561964e45ff02ffb58f48d7f11c58
inline bool test_041561964e45ff02ffb58f48d7f11c58(const int* i) {
    return i != nullptr;
}
#endif
