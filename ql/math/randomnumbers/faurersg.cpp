/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Gianni Piolanti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/randomnumbers/faurersg.hpp>
#include <ql/math/primenumbers.hpp>

namespace QuantLib {

    FaureRsg::FaureRsg(Size dimensionality)
    : dimensionality_(dimensionality),
//      sequenceCounter_(0),
      sequence_(std::vector<Real> (dimensionality), 1.0),
      integerSequence_(dimensionality, 0) {

        QL_REQUIRE(dimensionality>0, 
                   "dimensionality must be greater than 0");

        // base is the lowest prime number >= dimensionality_
        Size i, j, k=1;
        base_=2;
        while (base_<dimensionality_) {
            base_ = (Size)PrimeNumbers::get(k);
            k++;
        }


        mbit_=(Size)(std::log((double)std::numeric_limits<long int>::max())/
            std::log((double)base_));
        gray_ = std::vector<std::vector<long int> >(dimensionality_,
            std::vector<long int>(mbit_+1, 0));
        bary_ = std::vector<long int>(mbit_+1, 0);


        //setMatrixValues();
        powBase_ = std::vector<std::vector<long int> >(mbit_,
            std::vector<long int>(2*base_-1, 0));
        powBase_[mbit_-1][base_] = 1;
        for (int i2=mbit_-2; i2>=0; --i2)
            powBase_[i2][base_] = powBase_[i2+1][base_] * base_;
        for (int ii=0; ii<(int)mbit_; ii++) {
            for (int j1=base_+1; j1<2*(int)base_-1; j1++ )
                powBase_[ii][j1] = powBase_[ii][j1-1] + powBase_[ii][base_];
            for (int j2=base_-1; j2>=0; --j2)
                powBase_[ii][j2] = powBase_[ii][j2+1] - powBase_[ii][base_];
        }

        addOne_.resize(base_);
        for (j=0; j<base_ ; j++)
            addOne_[j] = (j+1) % base_;


        //setPascalMatrix();
        for (k=0; k<mbit_; k++) {
            std::vector<std::vector<long int> > mm(dimensionality_+1,
                std::vector<long int>(k+1, 0));
            pascal3D.push_back(mm);
            pascal3D[k][0][k] = 1;
            pascal3D[k][1][0] = 1;
            pascal3D[k][1][k] = 1;
        }

        long int p1, p2;
        for (k=2; k<mbit_ ; k++) {
            for (i=1; i<k ; i++) {
                p1 = pascal3D[k-1][1][i-1];
                p2 = pascal3D[k-1][1][i];
                pascal3D[k][1][i] = (p1+p2) % base_;
            }
        }

        long int fact = 1, diag;
        for (j=2; j<dimensionality_; j++) {
          for (long int kk=mbit_-1; kk>=0 ; --kk) {
              diag = mbit_ - kk - 1;
              if (diag==0)
                  fact = 1;
              else
                  fact = (fact*j) % base_;
              for (long int ii=0; ii<=kk; ii++)
                  pascal3D[diag+ii][j][ii] = (fact*
                    pascal3D[diag+ii][1][ii]) % base_;
          }
        }


        normalizationFactor_ = (double)base_ * (double)powBase_[0][base_];
        // std::cout << IntegerFormatter::toString(dimensionality_) << ", " ;
        // std::cout << IntegerFormatter::toString(normalizationFactor_);
        // std::cout << std::endl;
  }

    void FaureRsg::generateNextIntSequence() const {
        // sequenceCounter_++;

        Size bit = 0;
        bary_[bit] = addOne_[bary_[bit]];
        while (bary_[bit] == 0) {
            bit++;
            bary_[bit] = addOne_[bary_[bit]];
        };
        QL_REQUIRE(bit != mbit_,
            "Error processing Faure sequence." );

        long int tmp, g1, g2;
        for (Size i=0; i<dimensionality_ ; i++) {
            for (Size j=0; j<=bit ; j++) {
                tmp = gray_[i][j];
                gray_[i][j] = (pascal3D[bit][i][j] + tmp) % base_;
                g1 = gray_[i][j];
                g2 = base_ - 1 + g1 - tmp;
                integerSequence_[i] += powBase_[j][g2];
            }
        }
    }

}


