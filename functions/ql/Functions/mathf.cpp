/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Functions/mathf.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/bicubicsplineinterpolation.hpp>
#include <ql/Math/primenumbers.hpp>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    Real interpolate2D(const std::vector<Real>& x_values,
                       const std::vector<Real>& y_values,
                       const Matrix& dataMatrix,
                       Real x, Real y, Integer interpolation2DType,
                       bool allowExtrapolation) {

        Real result = 0.0;

        switch (interpolation2DType) {
          case 1:
            result = BilinearInterpolation(x_values.begin(), x_values.end(),
                        y_values.begin(), y_values.end(), dataMatrix)(x,y,
                        allowExtrapolation);
            break;
          case 2:
            result = BicubicSpline(x_values.begin(), x_values.end(),
                        y_values.begin(), y_values.end(), dataMatrix)(x,y,
                        allowExtrapolation);
            break;
          default:
            QL_FAIL("invalid interpolation type");
        }

        return result;
    }

    Real normDist(Real x, Real mean, Real standard_dev,
                  bool cumulative) {
        if (cumulative) {
            return CumulativeNormalDistribution(mean, standard_dev)(x);
        } else {
            return NormalDistribution(mean, standard_dev)(x);
        }
    }


    Real normInv(Real probability, Real mean, Real standard_dev) {
        return InverseCumulativeNormal(mean, standard_dev)
            (probability);
    }

    Size primeNumbers(Size absoluteIndex) {
        return PrimeNumbers::get(absoluteIndex);
    }


    static MersenneTwisterUniformRng rng;

    Real rand() {
        return rng.next().value;
    }
    void randomize(BigNatural seed) {
        rng = MersenneTwisterUniformRng(seed);
    }

}
