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

        template <class I1, class I2, class M>
        class BicubicSplineImpl
            : public Interpolation2D::templateImpl<I1,I2,M> {
          public:
            BicubicSplineImpl(const I1& xBegin, const I1& xEnd,
                              const I2& yBegin, const I2& yEnd, const M& zData)
            : Interpolation2D::templateImpl<I1,I2,M>(xBegin,xEnd,
                                                     yBegin,yEnd,
                                                     zData) {
                calculate();
            }
            void calculate() {
                splines_.reserve(this->zData_.rows());
                for (Size i=0; i<(this->zData_.rows()); ++i)
                    splines_.push_back(CubicInterpolation(
                                this->xBegin_, this->xEnd_,
                                this->zData_.row_begin(i),
                                CubicInterpolation::Spline, false,
                                CubicInterpolation::SecondDerivative, 0.0,
                                CubicInterpolation::SecondDerivative, 0.0));
            }
            Real value(Real x, Real y) const {
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
          private:
            std::vector<Interpolation> splines_;
        };

    }

    //! bicubic-spline interpolation between discrete points
    /*! \todo revise end conditions */
    class BicubicSpline : public Interpolation2D {
      public:
        /*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
        template <class I1, class I2, class M>
        BicubicSpline(const I1& xBegin, const I1& xEnd,
                      const I2& yBegin, const I2& yEnd,
                      const M& zData) {
            impl_ = boost::shared_ptr<Interpolation2D::Impl>(
                  new detail::BicubicSplineImpl<I1,I2,M>(xBegin, xEnd,
                                                         yBegin, yEnd, zData));
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
