
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bilinearinterpolation.hpp
    \brief bilinear interpolation between discrete points
*/

#ifndef quantlib_bilinear_interpolation_hpp
#define quantlib_bilinear_interpolation_hpp

#include <ql/Math/interpolation2D.hpp>

namespace QuantLib {

    //! bilinear interpolation between discrete points
    class BilinearInterpolation : public Interpolation2D {
      protected:
        //! bilinear interpolation implementation
        template <class I1, class I2, class M>
        class Impl : public Interpolation2D::templateImpl<I1,I2,M> {
          public:
            Impl(const I1& xBegin, const I1& xEnd, 
                 const I2& yBegin, const I2& yEnd, const M& zData)
            : Interpolation2D::templateImpl<I1,I2,M>(xBegin,xEnd,
                                                     yBegin,yEnd,
                                                     zData) {}
            Real value(Real x, Real y) const {
                Size i = locateX(x), j = locateY(y);

                Real z1=zData_[j][i];
                Real z2=zData_[j][i+1];
                Real z3=zData_[j+1][i];
                Real z4=zData_[j+1][i+1];

                Real t=(x-xBegin_[i])/(xBegin_[i+1]-xBegin_[i]);
                Real u=(y-yBegin_[j])/(yBegin_[j+1]-yBegin_[j]);

                return (1.0-t)*(1.0-u)*z1 + t*(1.0-u)*z2 
                     + (1.0-t)*u*z3 + t*u*z4;
            }
        };
      public:
        /*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
        template <class I1, class I2, class M>
        BilinearInterpolation(const I1& xBegin, const I1& xEnd, 
                              const I2& yBegin, const I2& yEnd,
                              const M& zData) {
            impl_ = boost::shared_ptr<Interpolation2D::Impl>(
                  new BilinearInterpolation::Impl<I1,I2,M>(xBegin, xEnd, 
                                                           yBegin, yEnd,
                                                           zData));
        }
    };

}


#endif
