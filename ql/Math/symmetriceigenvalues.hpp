
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file symmetriceigenvalues.hpp
    \brief Eigenvalues / eigenvectors of a real symmetric matrix
*/

// $Id$

#ifndef quantlib_symmetric_eigenvalues_h
#define quantlib_symmetric_eigenvalues_h

#include <ql/Math/symmetricschurdecomposition.hpp>

namespace QuantLib {

    namespace Math {

        // Eigenvalues of a symmetric matrix
        Array SymmetricEigenvalues(Matrix &s);

        // Eigenvectors of a symmetric matrix
        Matrix SymmetricEigenvectors(Matrix &s);


        // inline definitions

        inline Array SymmetricEigenvalues(Matrix &s) {
            return SymmetricSchurDecomposition(s).eigenvalues();
        }

        inline Matrix SymmetricEigenvectors(Matrix &s) {
            return SymmetricSchurDecomposition(s).eigenvectors();
        }

    }

}

#endif


