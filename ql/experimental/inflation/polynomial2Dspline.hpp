/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl
 Copyright (C) 2009 Bernd Engelmann

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

/*! \file polynomial2Dspline.hpp
    \brief polynomial interpolation in the y-direction, spline interpolation x-direction
*/

#ifndef quantlib_polynomial2D_spline_hpp
#define quantlib_polynomial2D_spline_hpp

#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2, class M>
        class Polynomial2DSplineImpl
            : public Interpolation2D::templateImpl<I1,I2,M> {
          public:
            Polynomial2DSplineImpl(const I1& xBegin, const I1& xEnd,
                                   const I2& yBegin, const I2& yEnd, const M& zData)
            : Interpolation2D::templateImpl<I1,I2,M>(xBegin,xEnd,
                                                     yBegin,yEnd,
                                                     zData) {
                calculate();
            }
            void calculate() {
                QL_REQUIRE(this->zData_.rows() == this->yEnd_ - this->yBegin_,
                    "size mismatch of the interpolation data");

                polynomials_.reserve(this->zData_.columns());
                for (Size i=0; i<(this->zData_.columns()); ++i)
                    polynomials_.push_back(Parabolic(
                        this->yBegin_, this->yEnd_,
                        this->zData_.column_begin(i)));
            }
            Real value(Real x,
                       Real y) const {
                std::vector<Real> section(polynomials_.size());
                for (Size i=0; i<polynomials_.size(); ++i)
                    section[i] = polynomials_[i](y, true);

                QL_REQUIRE(section.size() == this->xEnd_ - this->xBegin_,
                           "size mismatch of the interpolation data");

                CubicInterpolation spline(
                    this->xBegin_, this->xEnd_,
                    section.begin(),
                    CubicInterpolation::Spline, true,
                    CubicInterpolation::SecondDerivative, 0.0,
                    CubicInterpolation::SecondDerivative, 0.0);
                return spline(x,true);
            }
          private:
            std::vector<Interpolation> polynomials_;
        };

    }

    //! polynomial2D-spline interpolation between discrete points
    class Polynomial2DSpline : public Interpolation2D {
      public:
        /*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
        template <class I1, class I2, class M>
        Polynomial2DSpline(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin, const I2& yEnd,
                           const M& zData) {
            impl_ = std::shared_ptr<Interpolation2D::Impl>(
                  new detail::Polynomial2DSplineImpl<I1,I2,M>(xBegin, xEnd,
                                                              yBegin, yEnd, zData));
        }
    };

    //! polynomial2D-spline-interpolation factory
    class Polynomial {
      public:
        template <class I1, class I2, class M>
        Interpolation2D interpolate(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin, const I2& yEnd,
                                    const M& z) const {
            return Polynomial2DSpline(xBegin,xEnd,yBegin,yEnd,z);
        }
    };

}

#endif
