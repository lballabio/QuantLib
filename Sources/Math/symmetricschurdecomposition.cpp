
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

/*! \file symmetricschurdecomposition.cpp
    \brief Gives the eigenvalues and eigenvectors of a real symmetric matrix

    $Source$
    $Name$
    $Log$
    Revision 1.6  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.5  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.4  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.3  2001/02/16 15:14:07  lballabio
    renamed sqrt to matrixSqrt

    Revision 1.2  2001/01/25 11:57:33  lballabio
    Included outer product and sqrt into matrix.h

    Revision 1.1  2001/01/25 10:29:18  marmar
    JacobiDecomposition renamed SymmetricSchurDecomposition

    Revision 1.1  2001/01/24 13:00:26  marmar
    Jacobi decomposition return eigenvalues and eigenvectors
    of a square symmmetric real matrix

*/

/*
    Note: because of the many levels of indentation required,
    only 2 spaces are used for each indentation
*/

#include "Math/symmetricschurdecomposition.hpp"

namespace QuantLib {

  namespace Math {

    SymmetricSchurDecomposition::SymmetricSchurDecomposition(Matrix & s)
          : s_(s), hasBeenComputed_(false), size_(s.rows()),
          diagonal_(s.rows()), maxIterations_(100), epsPrec_(1e-15),
          eigenVectors_(s.rows(),s.columns(),0){

      QL_REQUIRE(s.rows() == s.columns(),
        "SymmetricSchurDecomposition: input matrix must be square");

      for(int ite = 0; ite < size_; ite++){
          diagonal_[ite] = s[ite][ite];
          eigenVectors_[ite][ite] = 1.0;
      }

    }

    void SymmetricSchurDecomposition::compute() const{

      double threshold;
      Array tmpDiag(diagonal_);
      Array tmpAccumulate(size_,0);
      Matrix s(s_);
      bool keeplooping = true;

      int ite = 1;
      do{ //main loop
        double sum = 0;
        for (int j = 0; j < size_-1; j++) {
            for (int k = j + 1; k < size_; k++){
                sum += QL_FABS(s[j][k]);
            }
        }

        if (sum == 0){
          keeplooping = false;
        }
        else{
        /*! To speed up computation a threshold is introduced to
           make sure it is worthy to perform the Jacobi rotation
        */
          if (ite < 5)
            threshold = 0.2*sum/(size_*size_);
          else
            threshold = 0;

		  int j;
          for (j = 0; j < size_-1; j++) {
            for (int k = j+1; k < size_; k++) {

              double sine,rho,cosin,heig,tang,beta;
              double smll = QL_FABS(s[j][k]);
              if( ite > 5 &&
                  smll < epsPrec_ * QL_FABS(diagonal_[j]) &&
                  smll < epsPrec_ * QL_FABS(diagonal_[k]))
    	          s[j][k] = 0;
              else if (QL_FABS(s[j][k]) > threshold) {
                heig = diagonal_[k]-diagonal_[j];
                if ( smll < epsPrec_ * QL_FABS(heig) )
    	            tang = s[j][k]/heig;
                else {
    	            beta = 0.5*heig/s[j][k];
    	            tang = 1/(QL_FABS(beta)+QL_SQRT(1+beta*beta));
    	            if(beta < 0) tang = -tang;
                }
                cosin = 1/QL_SQRT(1+tang*tang);
                sine = tang*cosin;
                rho = sine/(1+cosin);
                heig=tang*s[j][k];
                tmpAccumulate[j] -= heig;
                tmpAccumulate[k] += heig;
                diagonal_[j] -= heig;
                diagonal_[k] += heig;
                s[j][k] = 0;
				int l;
                for (l = 0; l <= j-1; l++) {
    	            jacobiRotate(s,rho,sine,l,j,l,k);
                }
                for (l = j+1; l <= k-1; l++) {
    	            jacobiRotate(s,rho,sine,j,l,l,k);
                }
                for (l = k+1; l < size_; l++) {
    	            jacobiRotate(s,rho,sine,j,l,k,l);
                }
                for (l = 0; l < size_; l++) {
    	            jacobiRotate(eigenVectors_,rho,sine,l,j,l,k);
                }
              }
            }
          }
          for (j = 0; j < size_; j++) {
            tmpDiag[j] += tmpAccumulate[j];
            diagonal_[j] = tmpDiag[j];
            tmpAccumulate[j] = 0;
          }
        }
      }while(++ite <= maxIterations_ && keeplooping);
      QL_REQUIRE(ite <= maxIterations_,
          "SymmetricSchurDecomposition::compute: Too many iterations reached");
      hasBeenComputed_ = true;
    }// end of method compute

  }

}
