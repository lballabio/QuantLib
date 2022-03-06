/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file laplaceinterpolation.hpp
    \brief Laplace interpolation of missing values
*/

#ifndef quantlib_laplace_interpolation
#define quantlib_laplace_interpolation

#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/matrixutilities/sparsematrix.hpp>

namespace QuantLib {

/*! reference: Numerical Recipes, 3rd edition, ch. 3.8
    two dimensional reconstruction of missing (i.e. null)
    values using laplace interpolation assuming an
    equidistant grid */

template <class M> void laplaceInterpolation(M &A, Real relTol = 1E-6) {

    struct f_A {
        const SparseMatrix &g;
        explicit f_A(const SparseMatrix &g) : g(g) {}
        Disposable<Array> operator()(const Array &x) const {
            return prod(g, x);
        }
    };

    Size m = A.rows();
    Size n = A.columns();

    QL_REQUIRE(n > 1 && m > 1, "matrix (" << m << "," << n
                                          << ") must at least be 2x2");

    SparseMatrix g(m * n, m * n, 5 * m * n);
    Array rhs(m * n, 0.0), guess(m * n, 0.0);
    Real guessTmp = 0.0;
    Size i1, i2, i3, i4, j1, j2, j3, j4;
    bool inner;

    for (Size l = 0, i = 0; i < m; ++i) {
        for (Size j = 0; j < n; ++j) {

            inner = false;

            // top
            if (i == 0) {
                if (j == 0) {
                    i1 = 0;
                    j1 = 1;
                    i2 = 1;
                    j2 = 0;
                } else {
                    if (j == n - 1) {
                        i1 = 0;
                        j1 = n - 2;
                        i2 = 1;
                        j2 = n - 1;
                    } else {
                        i1 = i2 = 0;
                        j1 = j - 1;
                        j2 = j + 1;
                    }
                }
            }

            // bottom
            if (i == m - 1) {
                if (j == 0) {
                    i1 = m - 1;
                    j1 = 1;
                    i2 = m - 2;
                    j2 = 0;
                } else {
                    if (j == n - 1) {
                        i1 = m - 1;
                        j1 = n - 2;
                        i2 = m - 2;
                        j2 = n - 1;
                    } else {
                        i1 = i2 = m - 1;
                        j1 = j - 1;
                        j2 = j + 1;
                    }
                }
            }

            // left / right
            if (i > 0 && i < m - 1) {
                if (j == 0 || j == n - 1) {
                    j1 = j2 = j;
                    i1 = i - 1;
                    i2 = i + 1;
                } else {
                    inner = true;
                    i1 = i - 1;
                    i2 = i - 1;
                    i3 = i + 1;
                    i4 = i + 1;
                    j1 = j - 1;
                    j2 = j + 1;
                    j3 = j - 1;
                    j4 = j + 1;
                }
            }

            g(l, i * n + j) = 1.0;
            if (A[i][j] == Null<Real>()) {
                if (inner) {
                    g(l, i1 * n + j1) = -0.25;
                    g(l, i2 * n + j2) = -0.25;
                    g(l, i3 * n + j3) = -0.25;
                    g(l, i4 * n + j4) = -0.25;
                } else {
                    g(l, i1 * n + j1) = -0.5;
                    g(l, i2 * n + j2) = -0.5;
                }
                rhs[l] = 0.0;
                guess[l] = guessTmp;
            } else {
                rhs[l] = A[i][j];
                guess[l] = guessTmp = A[i][j];
            }
            ++l;
        }
    }

    // solve the equation (preconditioner is identiy)
    Array s = BiCGstab(f_A(g), 10 * m * n, relTol)
                  .solve(rhs, guess)
                  .x;

    // replace missing values by solution
    for (Size i = 0; i < m; ++i) {
        for (Size j = 0; j < n; ++j) {
            if (A[i][j] == Null<Real>()) {
                A[i][j] = s[i * n + j];
            }
        }
    }
};

} // namespace QuantLib

#endif // include guard


#ifndef id_a5d16d0395520aea756da853957e1d91
#define id_a5d16d0395520aea756da853957e1d91
inline bool test_a5d16d0395520aea756da853957e1d91(int* i) { return i != 0; }
#endif
