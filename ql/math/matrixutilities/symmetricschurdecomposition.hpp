/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file symmetricschurdecomposition.hpp
    \brief Eigenvalues/eigenvectors of a real symmetric matrix
*/

#ifndef quantlib_math_jacobi_decomposition_h
#define quantlib_math_jacobi_decomposition_h

#include <ql/math/matrix.hpp>

namespace QuantLib {

    //! symmetric threshold Jacobi algorithm.
    /*! Given a real symmetric matrix S, the Schur decomposition
        finds the eigenvalues and eigenvectors of S. If D is the
        diagonal matrix formed by the eigenvalues and U the
        unitarian matrix of the eigenvectors we can write the
        Schur decomposition as
        \f[ S = U \cdot D \cdot U^T \, ,\f]
        where \f$ \cdot \f$ is the standard matrix product
        and  \f$ ^T  \f$ is the transpose operator.
        This class implements the Schur decomposition using the
        symmetric threshold Jacobi algorithm. For details on the
        different Jacobi transfomations see "Matrix computation,"
        second edition, by Golub and Van Loan,
        The Johns Hopkins University Press

        \test the correctness of the returned values is tested by
              checking their properties.
    */
    class SymmetricSchurDecomposition {
      public:
        /*! \pre s must be symmetric */
        SymmetricSchurDecomposition(const Matrix &s);
        const Array& eigenvalues() const { return diagonal_; }
        const Matrix& eigenvectors() const { return eigenVectors_; }
      private:
        Array diagonal_;
        Matrix eigenVectors_;
        void jacobiRotate_(Matrix & m, Real rot, Real dil,
                           Size j1, Size k1, Size j2, Size k2) const;
    };


    // inline definitions

    //! This routines implements the Jacobi, a.k.a. Givens, rotation
    inline void SymmetricSchurDecomposition::jacobiRotate_(
                                      Matrix &m, Real rot, Real dil, Size j1,
                                      Size k1, Size j2, Size k2) const {
        Real x1, x2;
        x1 = m[j1][k1];
        x2 = m[j2][k2];
        m[j1][k1] = x1 - dil*(x2 + x1*rot);
        m[j2][k2] = x2 + dil*(x1 - x2*rot);
    }

}


#endif




#ifndef id_2288ea5a27f5d1d5ceb1c1e2030c54d4
#define id_2288ea5a27f5d1d5ceb1c1e2030c54d4
inline bool test_2288ea5a27f5d1d5ceb1c1e2030c54d4(int* i) { return i != 0; }
#endif
