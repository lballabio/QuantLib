/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

/*! \file bilinearinterpolation.hpp
    \brief bilinear interpolation between discrete points
*/

#ifndef quantlib_bilinear_interpolation_hpp
#define quantlib_bilinear_interpolation_hpp

#include <ql/math/interpolations/interpolation2d.hpp>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2, class M>
        class BilinearInterpolationImpl
            : public Interpolation2D::templateImpl<I1,I2,M> {
          public:
            BilinearInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                      const I2& yBegin, const I2& yEnd,
                                      const M& zData)
            : Interpolation2D::templateImpl<I1,I2,M>(xBegin,xEnd,
                                                     yBegin,yEnd,
                                                     zData) {
                calculate();
            }
            void calculate() {}
            Real value(Real x, Real y) const {
                Size i = this->locateX(x), j = this->locateY(y);

                Real z1 = this->zData_[j][i];
                Real z2 = this->zData_[j][i+1];
                Real z3 = this->zData_[j+1][i];
                Real z4 = this->zData_[j+1][i+1];

                Real t=(x-this->xBegin_[i])/
                    (this->xBegin_[i+1]-this->xBegin_[i]);
                Real u=(y-this->yBegin_[j])/
                    (this->yBegin_[j+1]-this->yBegin_[j]);

                return (1.0-t)*(1.0-u)*z1 + t*(1.0-u)*z2
                     + (1.0-t)*u*z3 + t*u*z4;
            }
        };

    }

    //! %bilinear interpolation between discrete points
    class BilinearInterpolation : public Interpolation2D {
      public:
        /*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
        template <class I1, class I2, class M>
        BilinearInterpolation(const I1& xBegin, const I1& xEnd,
                              const I2& yBegin, const I2& yEnd,
                              const M& zData) {
            impl_ = boost::shared_ptr<Interpolation2D::Impl>(
                  new detail::BilinearInterpolationImpl<I1,I2,M>(xBegin, xEnd,
                                                                 yBegin, yEnd,
                                                                 zData));
        }
    };

    //! bilinear-interpolation factory
    class Bilinear {
      public:
        template <class I1, class I2, class M>
        Interpolation2D interpolate(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin, const I2& yEnd,
                                    const M& z) const {
            return BilinearInterpolation(xBegin,xEnd,yBegin,yEnd,z);
        }
    };

}


#endif
