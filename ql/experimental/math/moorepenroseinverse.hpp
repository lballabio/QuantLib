/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file moorepenroseinverse.hpp
    \brief Moore Penrose inverse of a real matrix
*/

#ifndef quantlib_moore_penrose_inverse
#define quantlib_moore_penrose_inverse

#include <ql/math/matrixutilities/svd.hpp>

namespace QuantLib {

/*! Reference:
    http://de.mathworks.com/help/matlab/ref/pinv.html
    https://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse */

inline Matrix moorePenroseInverse(const Matrix &A,
                                  const Real tol = Null<Real>()) {

    Size m = A.rows();
    Size n = A.columns();

    SVD svd(A);

    Real tol0 = tol;
    if (tol0 == Null<Real>()) {
        tol0 = std::max(m, n) * QL_EPSILON * std::abs(svd.singularValues()[0]);
    }

    Matrix sp(n, n, 0.0);
    for (Size i = 0; i < n; ++i) {
        if (std::abs(svd.singularValues()[i]) > tol0) {
            sp(i, i) = 1.0 / svd.singularValues()[i];
        }
    }

    Matrix res = svd.V() * sp * transpose(svd.U());
    return res;
};

} // namespace QuantLib

#endif // include guard
