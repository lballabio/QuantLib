
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

    //! %Linear interpolation traits
    class Linear {
      public:
        template <class I1, class I2>
        static Interpolation make_interpolation(const I1& xBegin, 
                                                const I1& xEnd,
                                                const I2& yBegin) {
            return LinearInterpolation(xBegin,xEnd,yBegin);
        }
        template <class I1, class I2, class M>
        static Interpolation2D make_interpolation(const I1& xBegin, 
                                                  const I1& xEnd,
                                                  const I2& yBegin, 
                                                  const I2& yEnd,
                                                  const M& z) {
            return BilinearInterpolation(xBegin,xEnd,yBegin,yEnd,z);
        }
    };

    //! Log-linear interpolation traits
    class LogLinear {
      public:
        template <class I1, class I2>
        static Interpolation make_interpolation(const I1& xBegin, 
                                                const I1& xEnd,
                                                const I2& yBegin) {
            return LogLinearInterpolation(xBegin,xEnd,yBegin);
        }
    };

    //! Cubic-spline interpolation traits
    class Cubic {
      public:
        template <class I1, class I2>
        static Interpolation make_interpolation(const I1& xBegin, 
                                                const I1& xEnd,
                                                const I2& yBegin) {
            return NaturalCubicSpline(xBegin,xEnd,yBegin);
        }
        template <class I1, class I2, class M>
        static Interpolation2D make_interpolation(const I1& xBegin, 
                                                  const I1& xEnd,
                                                  const I2& yBegin, 
                                                  const I2& yEnd,
                                                  const M& z) {
            return BicubicSpline(xBegin,xEnd,yBegin,yEnd,z);
        }
    };

}


#endif
