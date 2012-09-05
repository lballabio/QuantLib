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

/*! \file qrdecomposition.hpp
    \brief QR decomposition
*/

#ifndef quantlib_qr_decomposition_hpp
#define quantlib_qr_decomposition_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    //! QR decompoisition
    /*! This implementation is based on MINPACK
        (<http://www.netlib.org/minpack>,
        <http://www.netlib.org/cephes/linalg.tgz>)

        This subroutine uses householder transformations with column
        pivoting (optional) to compute a qr factorization of the
        m by n matrix A. That is, qrfac determines an orthogonal
        matrix q, a permutation matrix p, and an upper trapezoidal
        matrix r with diagonal elements of nonincreasing magnitude,
        such that A*p = q*r.

        Return value ipvt is an integer array of length n, which
        defines the permutation matrix p such that A*p = q*r.
        Column j of p is column ipvt(j) of the identity matrix.

        See lmdiff.cpp for further details.
    */
    Disposable<std::vector<Size> > qrDecomposition(const Matrix& A,
                                                   Matrix& q,
                                                   Matrix& r,
                                                   bool pivot = true);

    //! QR Solve
    /*! This implementation is based on MINPACK
        (<http://www.netlib.org/minpack>,
        <http://www.netlib.org/cephes/linalg.tgz>)

        Given an m by n matrix A, an n by n diagonal matrix d,
        and an m-vector b, the problem is to determine an x which
        solves the system

        A*x = b ,     d*x = 0 ,

        in the least squares sense.

        d is an input array of length n which must contain the
        diagonal elements of the matrix d.

        See lmdiff.cpp for further details.
    */
    Disposable<Array> qrSolve(const Matrix& a,
                              const Array& b,
                              bool pivot = true,
                              const Array& d = Array());
}

#endif
