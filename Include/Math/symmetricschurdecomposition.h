/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file symmetricschurdecomposition.h
    \brief Gives the eigenvalues and eigenvectors of a real symmetric matrix

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/25 10:21:51  marmar
    JacobiDecomposition renamed SymmetricSchurDecomposition

    Revision 1.1  2001/01/24 13:00:39  marmar
    Jacobi decomposition return eigenvalues and eigenvectors
    of a square symmmetric real matrix

*/


#ifndef quantlib_math_jacobi_decomposition_h
#define quantlib_math_jacobi_decomposition_h

#include "matrix.h"

namespace QuantLib {

    namespace Math {
      //! Implementing the symmetric threshold Jacobi algorithm
      /*! Given a real symmetric matrix S, the Schur decomposition 
          finds the eigenvalues and eigenvectors of S. If D is the 
          diagonal matrix formed by the eigenvalues and U the
          unitarian matrix of th eigenvector we can write the
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
            SymmetricSchurDecomposition(Matrix &s); // s must be symmetric
            Array eigenvalues() const;
            Matrix eigenvectors() const;
        private:
            int size_;
            int maxIterations_;
            double epsPrec_;
            Matrix s_;
            mutable bool hasBeenComputed_;  
            mutable Array diagonal_;
            mutable Matrix eigenVectors_; 
            void compute() const;
            void jacobiRotate(Matrix & m, double rot, double dil,
                int j1, int k1, int j2, int k2) const;
        };
        
        inline Array SymmetricSchurDecomposition::eigenvalues() const{
            if(!hasBeenComputed_)
                compute();
            return diagonal_;
        }
        
        inline Matrix SymmetricSchurDecomposition::eigenvectors() const{
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


