/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file gaussianquadratures.hpp
    \brief Integral of a 1-dimensional function using the Gauss quadratures
*/

#include <ql/Math/gaussianquadratures.hpp>
#include <ql/Math/tqreigendecomposition.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>

namespace QuantLib {

    GaussianQuadrature::GaussianQuadrature(
                                 Size n,
                                 const GaussianOrthogonalPolynomial& orthPoly)
    : x_(n), w_(n) {

        // set-up matrix to compute the roots and the weights
        Array e(n-1);

        Size i;
        for (i=1; i < n; ++i) {
            x_[i] = orthPoly.alpha(i);
            e[i-1] = std::sqrt(orthPoly.beta(i));
        }
        x_[0] = orthPoly.alpha(0);

        TqrEigenDecomposition tqr(
                               x_, e,
                               TqrEigenDecomposition::OnlyFirstRowEigenVector,
                               TqrEigenDecomposition::Overrelaxation);

        x_ = tqr.eigenvalues();
        const Matrix& ev = tqr.eigenvectors();

        Real mu_0 = orthPoly.mu_0();
        for (i=0; i<n; ++i) {
            w_[i] = mu_0*ev[0][i]*ev[0][i] / orthPoly.w(x_[i]);
        }
    }

}

