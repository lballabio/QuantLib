/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file symmetriceigenvalues.hpp
    \brief Eigenvalues / eigenvectors of a real symmetric matrix
*/

#ifndef quantlib_symmetric_eigenvalues_h
#define quantlib_symmetric_eigenvalues_h

#include <ql/Math/symmetricschurdecomposition.hpp>

namespace QuantLib {

    // Eigenvalues of a symmetric matrix
    Disposable<Array> SymmetricEigenvalues(Matrix &s);

    // Eigenvectors of a symmetric matrix
    Disposable<Matrix> SymmetricEigenvectors(Matrix &s);


    // inline definitions

    inline Disposable<Array> SymmetricEigenvalues(Matrix &s) {
        Array result = SymmetricSchurDecomposition(s).eigenvalues();
        return result;
    }

    inline Disposable<Matrix> SymmetricEigenvectors(Matrix &s) {
        Matrix result = SymmetricSchurDecomposition(s).eigenvectors();
        return result;
    }

}


#endif
