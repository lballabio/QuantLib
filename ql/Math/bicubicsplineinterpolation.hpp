
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file bicubicsplineinterpolation.hpp
    \brief bicubic spline interpolation between discrete points
*/

#ifndef quantlib_bicubic_spline_interpolation_hpp
#define quantlib_bicubic_spline_interpolation_hpp

#include <ql/Math/interpolation2D.hpp>
#include <ql/Math/cubicspline.hpp>

namespace QuantLib {

    /*! bicubic spline interpolation between discrete points
        \todo revise end conditions
    */
    class BicubicSpline : public Interpolation2D {
      protected:
        //! bicubic spline implementation
        template <class I1, class I2, class M>
        class Impl : public Interpolation2D::templateImpl<I1,I2,M> {
          public:
            Impl(const I1& xBegin, const I1& xEnd, 
                 const I2& yBegin, const I2& yEnd, const M& zData)
            : Interpolation2D::templateImpl<I1,I2,M>(xBegin,xEnd,
                                                     yBegin,yEnd,
                                                     zData) {
                for (Size i=0; i<zData_.rows(); i++)
                    splines_.push_back(NaturalCubicSpline(
                                           xBegin, xEnd, zData_.row_begin(i)));
            }
            double value(double x, double y) const {
                std::vector<double> section(splines_.size());
                for (Size i=0; i<splines_.size(); i++)
                    section[i]=splines_[i](x,true);

                NaturalCubicSpline spline(yBegin_, yEnd_, section.begin());
                return spline(y,true);
            }
          private:
            std::vector<Interpolation> splines_;
        };
      public:
        /*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
        template <class I1, class I2, class M>
        BicubicSpline(const I1& xBegin, const I1& xEnd, 
                      const I2& yBegin, const I2& yEnd,
                      const M& zData) {
            impl_ = boost::shared_ptr<Interpolation2D::Impl>(
                  new BicubicSpline::Impl<I1,I2,M>(xBegin, xEnd, 
                                                   yBegin, yEnd, zData));
        }
    };

}


#endif
