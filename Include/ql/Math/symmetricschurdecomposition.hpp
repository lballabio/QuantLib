
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file symmetricschurdecomposition.hpp
    \brief Gives the eigenvalues and eigenvectors of a real symmetric matrix

    $Source$
    $Log$
    Revision 1.3  2001/05/24 13:57:51  nando
    smoothing #include xx.hpp and cutting old Log messages

    Revision 1.2  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

*/


#ifndef quantlib_math_jacobi_decomposition_h
#define quantlib_math_jacobi_decomposition_h

#include "ql/Math/matrix.hpp"

namespace QuantLib {

    namespace Math {

        //! symmetric threshold Jacobi algorithm.
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
            /* \pre s must be symmetric */
            SymmetricSchurDecomposition(Matrix &s);
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


