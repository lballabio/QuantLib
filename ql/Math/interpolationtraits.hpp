
/*
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file interpolationtraits.hpp
    \brief traits classes for interpolation algorithms
*/

#ifndef quantlib_interpolation_traits_hpp
#define quantlib_interpolation_traits_hpp

#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/bicubicsplineinterpolation.hpp>

namespace QuantLib {

    //! linear interpolation traits
    class Linear {
      public:
        template <class I1, class I2>
        static Handle<Interpolation<I1,I2> >
        make_interpolation(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin) {
            return Handle<Interpolation<I1,I2> >(
                          new LinearInterpolation<I1,I2>(xBegin,xEnd,yBegin));
        }
        template <class I1, class I2, class M>
        static Handle<Interpolation2D<I1,I2,M> >
        make_interpolation(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin, const I2& yEnd,
                           const M& z) {
            return Handle<Interpolation2D<I1,I2,M> >(
                           new BilinearInterpolation<I1,I2,M>(xBegin,xEnd,
                                                              yBegin,yEnd,z));
        }
    };

    //! log-linear interpolation traits
    class LogLinear {
      public:
        template <class I1, class I2>
        static Handle<Interpolation<I1,I2> >
        make_interpolation(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin) {
            return Handle<Interpolation<I1,I2> >(
                       new LogLinearInterpolation<I1,I2>(xBegin,xEnd,yBegin));
        }
    };

    //! cubic-spline interpolation traits
    class CubicSpline {
      public:
        template <class I1, class I2>
        static Handle<Interpolation<I1,I2> >
        make_interpolation(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin) {
            return Handle<Interpolation<I1,I2> >(
                     new CubicSplineInterpolation<I1,I2>(xBegin,xEnd,yBegin,
                     Null<double>(), 0.0, Null<double>(), 0.0, false));
        }
        template <class I1, class I2, class M>
        static Handle<Interpolation2D<I1,I2,M> >
        make_interpolation(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin, const I2& yEnd,
                           const M& z) {
            return Handle<Interpolation2D<I1,I2,M> >(
                      new BicubicSplineInterpolation<I1,I2,M>(xBegin,xEnd,
                                                              yBegin,yEnd,z));
        }
    };

}


#endif
