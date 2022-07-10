/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file qrdecomposition.cpp
    \brief QR decomposition
*/

#include <ql/math/optimization/lmdif.hpp>
#include <ql/math/matrixutilities/qrdecomposition.hpp>
#include <memory>

namespace QuantLib {

    std::vector<Size> qrDecomposition(const Matrix& M,
                                      Matrix& q,
                                      Matrix& r,
                                      bool pivot) {
        Matrix mT = transpose(M);
        const Size m = M.rows();
        const Size n = M.columns();

        std::unique_ptr<int[]> lipvt(new int[n]);
        std::unique_ptr<Real[]> rdiag(new Real[n]);
        std::unique_ptr<Real[]> wa(new Real[n]);

        MINPACK::qrfac(m, n, mT.begin(), 0, (pivot)?1:0,
                       lipvt.get(), n, rdiag.get(), rdiag.get(), wa.get());
        if (r.columns() != n || r.rows() !=n)
            r = Matrix(n, n);

        for (Size i=0; i < n; ++i) {
            std::fill(r.row_begin(i), r.row_begin(i)+i, 0.0);
            r[i][i] = rdiag[i];
            if (i < m) {
                std::copy(mT.column_begin(i)+i+1, mT.column_end(i),
                          r.row_begin(i)+i+1);
            }
            else {
                std::fill(r.row_begin(i)+i+1, r.row_end(i), 0.0);
            }
        }

        if (q.rows() != m || q.columns() != n)
            q = Matrix(m, n);

        if (m > n) {
            std::fill(q.begin(), q.end(), 0.0);

            Integer u = std::min(n,m);
            for (Integer i=0; i < u; ++i)
                q[i][i] = 1.0;

            Array v(m);
            for (Integer i=u-1; i >=0; --i) {
                if (std::fabs(mT[i][i]) > QL_EPSILON) {
                    const Real tau = 1.0/mT[i][i];

                    std::fill(v.begin(), v.begin()+i, 0.0);
                    std::copy(mT.row_begin(i)+i, mT.row_end(i), v.begin()+i);

                    Array w(n, 0.0);
                    for (Size l=0; l < n; ++l)
                        w[l] += std::inner_product(
                            v.begin()+i, v.end(), q.column_begin(l)+i, Real(0.0));

                    for (Size k=i; k < m; ++k) {
                        const Real a = tau*v[k];
                        for (Size l=0; l < n; ++l)
                            q[k][l] -= a*w[l];
                    }
                }
            }
        }
        else {
            Array w(m);
            for (Size k=0; k < m; ++k) {
                std::fill(w.begin(), w.end(), 0.0);
                w[k] = 1.0;

                for (Size j=0; j < std::min(n, m); ++j) {
                    const Real t3 = mT[j][j];
                    if (t3 != 0.0) {
                        const Real t
                            = std::inner_product(mT.row_begin(j)+j, mT.row_end(j),
                                                 w.begin()+j, Real(0.0))/t3;
                        for (Size i=j; i<m; ++i) {
                            w[i]-=mT[j][i]*t;
                        }
                    }
                    q[k][j] = w[j];
                }
                std::fill(q.row_begin(k) + std::min(n, m), q.row_end(k), 0.0);
            }
        }

        std::vector<Size> ipvt(n);

        if (pivot) {
            std::copy(lipvt.get(), lipvt.get()+n, ipvt.begin());
        }
        else {
            for (Size i=0; i < n; ++i)
                ipvt[i] = i;
        }

        return ipvt;
    }

    Array qrSolve(const Matrix& a, const Array& b,
                  bool pivot, const Array& d) {
        const Size m = a.rows();
        const Size n = a.columns();

        QL_REQUIRE(b.size() == m, "dimensions of A and b don't match");
        QL_REQUIRE(d.size() == n || d.empty(),
                   "dimensions of A and d don't match");

        Matrix q(m, n), r(n, n);

        std::vector<Size> lipvt = qrDecomposition(a, q, r, pivot);

        std::unique_ptr<int[]> ipvt(new int[n]);
        std::copy(lipvt.begin(), lipvt.end(), ipvt.get());

        Matrix rT = transpose(r);

        std::unique_ptr<Real[]> sdiag(new Real[n]);
        std::unique_ptr<Real[]> wa(new Real[n]);

        Array ld(n, 0.0);
        if (!d.empty()) {
            std::copy(d.begin(), d.end(), ld.begin());
        }

        Array x(n);
        Array qtb = transpose(q)*b;

        MINPACK::qrsolv(n, rT.begin(), n, ipvt.get(),
                        ld.begin(), qtb.begin(),
                        x.begin(), sdiag.get(), wa.get());

        return x;
    }
}
