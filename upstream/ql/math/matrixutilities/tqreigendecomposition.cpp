/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file tqreigendecomposition.hpp
    \brief tridiag. QR eigen decompositions with implicit shift
*/

#include <ql/math/matrixutilities/tqreigendecomposition.hpp>
#include <vector>

namespace QuantLib {

    TqrEigenDecomposition::TqrEigenDecomposition(const Array& diag,
                                                 const Array& sub,
                                                 EigenVectorCalculation calc,
                                                 ShiftStrategy strategy)
    : d_(diag), ev_((calc == WithEigenVector)    ? d_.size() :
                    (calc == WithoutEigenVector) ? 0 :
                                                   1,
                    d_.size(),
                    0) {
        Size n = diag.size();

        QL_REQUIRE(n == sub.size()+1, "Wrong dimensions");

        Array e(n, 0.0);
        std::copy(sub.begin(),sub.end(),e.begin()+1);
        for (Size i=0; i < ev_.rows(); ++i) {
            ev_[i][i] = 1.0;
        }

        for (Size k=n-1; k >=1; --k) {
            while (!offDiagIsZero(k, e)) {
                Size l = k;
                while (--l > 0 && !offDiagIsZero(l,e)); // NOLINT(bugprone-inc-dec-in-conditions)
                iter_++;

                Real q = d_[l];
                if (strategy != NoShift) {
                    // calculated eigenvalue of 2x2 sub matrix of
                    // [ d_[k-1] e_[k] ]
                    // [  e_[k]  d_[k] ]
                    // which is closer to d_[k+1].
                    const Real t1 = std::sqrt(
                                          0.25*(d_[k]*d_[k] + d_[k-1]*d_[k-1])
                                          - 0.5*d_[k-1]*d_[k] + e[k]*e[k]);
                    const Real t2 = 0.5*(d_[k]+d_[k-1]);

                    const Real lambda =
                        (std::fabs(t2+t1 - d_[k]) < std::fabs(t2-t1 - d_[k]))?
                        Real(t2+t1) : Real(t2-t1);

                    if (strategy == CloseEigenValue) {
                        q-=lambda;
                    } else {
                        q-=((k==n-1)? 1.25 : 1.0)*lambda;
                    }
                }

                // the QR transformation
                Real sine = 1.0;
                Real cosine = 1.0;
                Real u = 0.0;

                bool recoverUnderflow = false;
                for (Size i=l+1; i <= k && !recoverUnderflow; ++i) {
                    const Real h = cosine*e[i];
                    const Real p = sine*e[i];

                    e[i-1] = std::sqrt(p*p+q*q);
                    if (e[i-1] != 0.0) {
                        sine = p/e[i-1];
                        cosine = q/e[i-1];

                        const Real g = d_[i-1]-u;
                        const Real t = (d_[i]-g)*sine+2*cosine*h;

                        u = sine*t;
                        d_[i-1] = g + u;
                        q = cosine*t - h;

                        for (Size j=0; j < ev_.rows(); ++j) {
                            const Real tmp = ev_[j][i-1];
                            ev_[j][i-1] = sine*ev_[j][i] + cosine*tmp;
                            ev_[j][i] = cosine*ev_[j][i] - sine*tmp;
                        }
                    } else {
                        // recover from underflow
                        d_[i-1] -= u;
                        e[l] = 0.0;
                        recoverUnderflow = true;
                    }
                }

                if (!recoverUnderflow) {
                    d_[k] -= u;
                    e[k] = q;
                    e[l] = 0.0;
                }
            }
        }

        // sort (eigenvalues, eigenvectors),
        // code taken from symmetricSchureDecomposition.cpp
        std::vector<std::pair<Real, std::vector<Real> > > temp(n);
        std::vector<Real> eigenVector(ev_.rows());
        for (Size i=0; i<n; i++) {
            if (ev_.rows() > 0)
                std::copy(ev_.column_begin(i),
                          ev_.column_end(i), eigenVector.begin());
            temp[i] = std::make_pair(d_[i], eigenVector);
        }
        std::sort(temp.begin(), temp.end(), std::greater<>());
        // first element is positive
        for (Size i=0; i<n; i++) {
            d_[i] = temp[i].first;
            Real sign = 1.0;
            if (ev_.rows() > 0 && temp[i].second[0]<0.0)
                sign = -1.0;
            for (Size j=0; j<ev_.rows(); ++j) {
                ev_[j][i] = sign * temp[i].second[j];
            }
        }
    }

    // see NR for abort assumption as it is
    // not part of the original Wilkinson algorithm
    bool TqrEigenDecomposition::offDiagIsZero(Size k, Array& e) {
        return std::fabs(d_[k-1])+std::fabs(d_[k])
            == std::fabs(d_[k-1])+std::fabs(d_[k])+std::fabs(e[k]);
    }

}
