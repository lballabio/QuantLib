
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

/*! \file symmetricschurdecomposition.cpp
    \brief Eigenvalues / eigenvectors of a real symmetric matrix
*/

/*
    Note: because of the many levels of indentation required,
    only 2 spaces are used for each indentation
*/

#include <ql/Math/symmetricschurdecomposition.hpp>

namespace QuantLib {

  namespace Math {

    SymmetricSchurDecomposition::SymmetricSchurDecomposition(const Matrix & s)
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

    void SymmetricSchurDecomposition::compute() const {

      double threshold;
      Array tmpDiag(diagonal_);
      Array tmpAccumulate(size_,0);
      bool keeplooping = true;

      int ite = 1;
      do{ //main loop
        double sum = 0;
        for (int j = 0; j < size_-1; j++) {
            for (int k = j + 1; k < size_; k++){
                sum += QL_FABS(s_[j][k]);
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
              double smll = QL_FABS(s_[j][k]);
              if( ite > 5 &&
                  smll < epsPrec_ * QL_FABS(diagonal_[j]) &&
                  smll < epsPrec_ * QL_FABS(diagonal_[k]))
                  s_[j][k] = 0;
              else if (QL_FABS(s_[j][k]) > threshold) {
                heig = diagonal_[k]-diagonal_[j];
                if ( smll < epsPrec_ * QL_FABS(heig) )
                    tang = s_[j][k]/heig;
                else {
                    beta = 0.5*heig/s_[j][k];
                    tang = 1/(QL_FABS(beta)+QL_SQRT(1+beta*beta));
                    if(beta < 0) tang = -tang;
                }
                cosin = 1/QL_SQRT(1+tang*tang);
                sine = tang*cosin;
                rho = sine/(1+cosin);
                heig=tang*s_[j][k];
                tmpAccumulate[j] -= heig;
                tmpAccumulate[k] += heig;
                diagonal_[j] -= heig;
                diagonal_[k] += heig;
                s_[j][k] = 0;
                int l;
                for (l = 0; l <= j-1; l++) {
                    jacobiRotate(s_,rho,sine,l,j,l,k);
                }
                for (l = j+1; l <= k-1; l++) {
                    jacobiRotate(s_,rho,sine,j,l,l,k);
                }
                for (l = k+1; l < size_; l++) {
                    jacobiRotate(s_,rho,sine,j,l,k,l);
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
