
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

#include <ql/Math/matrix.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>
#include <vector>

namespace QuantLib {

    template<class I1, class I2, class I3>
    std::vector<double> interpolate(const I1& xx_begin,
                                    const I1& xx_end,
                                    const I2& yy_begin,
                                    const I3& x_begin,
                                    const I3& x_end,
                                    int interpolationType,
                                    bool allowExtrapolation,
                                    CubicSpline::BoundaryCondition leftCondition,
                                    double leftConditionValue,
                                    CubicSpline::BoundaryCondition rightCondition,
                                    double rightConditionValue,
                                    bool monotonicityConstraint,
                                    int derivativeOrder) {

        std::vector<double> result(x_end-x_begin);
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
            QL_FAIL("interpolate: invalid interpolation type");
        }

        switch (derivativeOrder) {
          case -1:
              // should be f.primitive(.., allowExtrapolation)
              std::transform(x_begin, x_end, result.begin(), f);
            break;
          case 0:
              // should be f(.., allowExtrapolation)
              std::transform(x_begin, x_end, result.begin(), f);
            break;
          case 1:
              // should be f.derivative(.., allowExtrapolation)
              std::transform(x_begin, x_end, result.begin(), f);
            break;
          case 2:
              // should be f.secondDerivative(.., allowExtrapolation)
              std::transform(x_begin, x_end, result.begin(), f);
            break;
          default:
              QL_FAIL(IntegerFormatter::toString(derivativeOrder)
                  + " is an invalid derivative order");
        }

        return result;
    }


    double interpolate2D(const std::vector<double>& x_values,
                         const std::vector<double>& y_values,
                         const Matrix& dataMatrix,
                         double x,
                         double y,
                         int interpolation2DType,
                         bool allowExtrapolation);



    double normDist(double x,
                    double mean = 0.0,
                    double standard_dev = 1.0,
                    bool cumulative = false);
    double normInv(double probability,
                   double mean = 0.0,
                   double standard_dev = 1.0);

    Size primeNumbers(Size absoluteIndex);

    double rand();
    void randomize(unsigned long seed);

}


#endif
