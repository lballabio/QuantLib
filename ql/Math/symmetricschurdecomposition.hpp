
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file symmetricschurdecomposition.hpp
    \brief Eigenvalues / eigenvectors of a real symmetric matrix
*/

#ifndef quantlib_math_jacobi_decomposition_h
#define quantlib_math_jacobi_decomposition_h

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    namespace Math {

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
            different Jacobi transfomations you can start from the great book
            on matrix computations by Golub and Van Loan: Matrix computation,
            second edition The Johns Hopkins University Press
        */
        class SymmetricSchurDecomposition {
          public:
            /* \pre s must be symmetric */
            SymmetricSchurDecomposition(const Matrix &s);
            const Array& eigenvalues() const;
            const Matrix& eigenvectors() const;
          private:
            mutable Matrix s_;
            int size_;
            mutable Array diagonal_;
            mutable Matrix eigenVectors_;
            mutable bool hasBeenComputed_;
            int maxIterations_;
            double epsPrec_;
            void compute() const;
            void jacobiRotate(Matrix & m, double rot, double dil,
                int j1, int k1, int j2, int k2) const;
        };


        // inline definitions

        inline const Array& SymmetricSchurDecomposition::eigenvalues() const{
            if(!hasBeenComputed_)
                compute();
            return diagonal_;
        }

        inline const Matrix& SymmetricSchurDecomposition::eigenvectors() const{
            if(!hasBeenComputed_)
                compute();
            return eigenVectors_;
        }

        //! This routines implements the Jacobi, a.k.a. Givens, rotation
        inline void SymmetricSchurDecomposition::jacobiRotate(Matrix &m,
              double rot, double dil, int j1, int k1, int j2, int k2) const{

            double x1, x2;
            x1 = m[j1][k1];
            x2 = m[j2][k2];
            m[j1][k1] = x1 - dil*(x2 + x1*rot);
            m[j2][k2] = x2 + dil*(x1 - x2*rot);
        }

    }

}

#endif


