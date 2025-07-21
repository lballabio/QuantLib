/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file numericaldifferentiation.cpp */

#include <ql/methods/finitedifferences/operators/numericaldifferentiation.hpp>

#pragma push_macro("BOOST_DISABLE_ASSERTS")
#ifndef BOOST_DISABLE_ASSERTS
#define BOOST_DISABLE_ASSERTS
#endif
#include <boost/multi_array.hpp>
#pragma pop_macro("BOOST_DISABLE_ASSERTS")

#include <utility>

namespace QuantLib {

    namespace {
        Array calcOffsets(
            Real h, Size n, NumericalDifferentiation::Scheme scheme) {
            QL_REQUIRE(n > 1, "number of steps must be greater than one");

            Array retVal(n);
            switch (scheme) {
              case NumericalDifferentiation::Central:
                QL_REQUIRE(n > 2 && (n % 2),
                    "number of steps must be an odd number greater than two");
                for (Integer i=0; i < Integer(n); ++i)
                    retVal[i] = (i-Integer(n/2))*h;
                break;
              case NumericalDifferentiation::Backward:
                for (Size i=0; i < n; ++i)
                    retVal[i]=-(i*h);
                break;
              case NumericalDifferentiation::Forward:
                for (Size i=0; i < n; ++i)
                    retVal[i]=i*h;
                break;
              default:
                QL_FAIL("unknown numerical differentiation scheme");
            }

            return retVal;
        }

        // This is a C++ implementation of the algorithm/pseudo code in
        // B. Fornberg, 1998. Calculation of Weights
        //                    in Finite Difference Formulas
        // https://amath.colorado.edu/faculty/fornberg/Docs/sirev_cl.pdf
        Array calcWeights(const Array& x, Size M) {
            const Size N = x.size();
            QL_REQUIRE(N > M, "number of points must be greater "
                               "than the order of the derivative");

            QL_DEPRECATED_DISABLE_WARNING
            boost::multi_array<Real, 3>  d(boost::extents[M+1][N][N]);
            QL_DEPRECATED_ENABLE_WARNING
            d[0][0][0] = 1.0;
            Real c1 = 1.0;

            for (Size n=1; n < N; ++n) {
                Real c2 = 1.0;
                for (Size nu=0; nu < n; ++nu) {
                    const Real c3 = x[n] - x[nu];
                    c2*=c3;

                    for (Size m=0; m <= std::min(n, M); ++m) {
                        d[m][n][nu] = (x[n]*d[m][n-1][nu]
                             - ((m > 0)? Real(m*d[m-1][n-1][nu]) : 0.0))/c3;
                    }
                }

                for (Size m=0; m <= M; ++m) {
                    d[m][n][n] = c1/c2*( ((m > 0)? Real(m*d[m-1][n-1][n-1]) : 0.0) -
                        x[n-1]*d[m][n-1][n-1] );
                }
                c1=c2;
            }


            Array retVal(N);
            for (Size i=0; i < N; ++i) {
                retVal[i] = d[M][N-1][i];
            }
            return retVal;
        }
    }

    NumericalDifferentiation::NumericalDifferentiation(std::function<Real(Real)> f,
                                                       Size orderOfDerivative,
                                                       Array x_offsets)
    : offsets_(std::move(x_offsets)), w_(calcWeights(offsets_, orderOfDerivative)),
      f_(std::move(f)) {}


    NumericalDifferentiation::NumericalDifferentiation(std::function<Real(Real)> f,
                                                       Size orderOfDerivative,
                                                       Real stepSize,
                                                       Size steps,
                                                       Scheme scheme)
    : offsets_(calcOffsets(stepSize, steps, scheme)), w_(calcWeights(offsets_, orderOfDerivative)),
      f_(std::move(f)) {}
}
