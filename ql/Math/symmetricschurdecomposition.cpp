
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file symmetricschurdecomposition.cpp

    \fullpath
    Sources/Math/%symmetricschurdecomposition.cpp
    \brief Eigenvalues / eigenvectors of a real symmetric matrix

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:57:12  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.16  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.15  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.14  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.13  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.12  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.11  2001/07/13 14:23:11  sigmud
// removed a few gcc compile warnings
//
// Revision 1.10  2001/06/05 09:35:14  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.9  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

/*
    Note: because of the many levels of indentation required,
    only 2 spaces are used for each indentation
*/

#include "ql/Math/symmetricschurdecomposition.hpp"

namespace QuantLib {

  namespace Math {

    SymmetricSchurDecomposition::SymmetricSchurDecomposition(Matrix & s)
          : s_(s), size_(s.rows()), diagonal_(s.rows()),
	  eigenVectors_(s.rows(),s.columns(),0), hasBeenComputed_(false),
	  maxIterations_(100), epsPrec_(1e-15) {

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
