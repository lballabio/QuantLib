
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano

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

/*! \file mathf.hpp
    \brief math functions
*/

#ifndef quantlib_functions_math_h
#define quantlib_functions_math_h

#include <ql/Functions/qlfunctions.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>

namespace QuantLib {

    template<class I1, class I2, class I3>
    std::vector<Real> interpolate(const I1& xx_begin,
                                  const I1& xx_end,
                                  const I2& yy_begin,
                                  const I3& x_begin,
                                  const I3& x_end,
                                  Integer interpolationType,
                                  bool allowExtrapolation,
                                  CubicSpline::BoundaryCondition leftCondition,
                                  Real leftConditionValue,
                                  CubicSpline::BoundaryCondition rightCondition,
                                  Real rightConditionValue,
                                  bool monotonicityConstraint,
                                  Integer derivativeOrder) {

        Interpolation f;

        switch (interpolationType) {
          case 1:
            f = LinearInterpolation(xx_begin, xx_end, yy_begin);
            break;
          case 2:
            f = CubicSpline(xx_begin, xx_end, yy_begin,
                leftCondition, leftConditionValue,
                rightCondition, rightConditionValue,
                monotonicityConstraint);
            break;
          case 3:
            f = LogLinearInterpolation(xx_begin, xx_end, yy_begin);
            break;
          default:
            QL_FAIL("invalid interpolation type");
        }

        Size i, n = x_end-x_begin;
        std::vector<Real> result(n);
        switch (derivativeOrder) {
          case -1:
              for (i=0; i<n; i++) {
                  result[i] = f.primitive(*(x_begin+i),allowExtrapolation);
              }
              break;
          case 0:
              for (i=0; i<n; i++) {
                  result[i] = f(*(x_begin+i),allowExtrapolation);
              }
            break;
          case 1:
              for (i=0; i<n; i++) {
                  result[i] = f.derivative(*(x_begin+i),allowExtrapolation);
              }
            break;
          case 2:
              for (i=0; i<n; i++) {
                  result[i] = f.secondDerivative(*(x_begin+i),
                      allowExtrapolation); 
              }
            break;
          default:
              QL_FAIL(IntegerFormatter::toString(derivativeOrder)
                  + " is an invalid derivative order");
        }

        return result;
    }


    Real interpolate2D(const std::vector<Real>& x_values,
                       const std::vector<Real>& y_values,
                       const Matrix& dataMatrix,
                       Real x,
                       Real y,
                       Integer interpolation2DType,
                       bool allowExtrapolation);



    Real normDist(Real x,
                  Real mean = 0.0,
                  Real standard_dev = 1.0,
                  bool cumulative = false);
    Real normInv(Real probability,
                 Real mean = 0.0,
                 Real standard_dev = 1.0);

    Size primeNumbers(Size absoluteIndex);

    Real rand();
    void randomize(BigNatural seed);

}


#endif
