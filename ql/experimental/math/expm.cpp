/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Klaus Spanderen

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

/*! \file expm.cpp
    \brief matrix exponential
*/


#include <ql/experimental/math/expm.hpp>
#include <ql/math/ode/adaptiverungekutta.hpp>
#include <algorithm>
#include <numeric>
#include <utility>

namespace QuantLib {

    namespace {
        class MatrixVectorProductFct {
          public:
            explicit MatrixVectorProductFct(Matrix m) : m_(std::move(m)) {}

            // implements x = M*y
            std::vector<Real> operator()(Real t, const std::vector<Real>& y) {

                std::vector<Real> result(m_.rows());
                for (Size i=0; i < result.size(); i++) {
                    result[i] = std::inner_product(y.begin(), y.end(),
                                                   m_.row_begin(i), Real(0.0));
                }
                return result;
            }
          private:
            const Matrix m_;
        };
    }

    Matrix Expm(const Matrix& M, Real t, Real tol) {
        const Size n = M.rows();
        QL_REQUIRE(n == M.columns(), "Expm expects a square matrix");

        AdaptiveRungeKutta<> rk(tol);
        AdaptiveRungeKutta<>::OdeFct odeFct = MatrixVectorProductFct(M);

        Matrix result(n, n);
        for (Size i=0; i < n; ++i) {
            std::vector<Real> x0(n, 0.0);
            x0[i] = 1.0;

            const std::vector<Real> r = rk(odeFct, x0, 0.0, t);
            std::copy(r.begin(), r.end(), result.column_begin(i));
        }
        return result;
    }
}
