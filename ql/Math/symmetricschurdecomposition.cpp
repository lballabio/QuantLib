
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

#include <ql/Math/symmetricschurdecomposition.hpp>
#include <vector>

namespace QuantLib {

    SymmetricSchurDecomposition::SymmetricSchurDecomposition(
        const Matrix & s)
    : diagonal_(s.rows()), eigenVectors_(s.rows(), s.columns(), 0.0) {

        Size size = s.rows();
        QL_REQUIRE(size==s.columns(),
                   "input matrix must be square");

        for (Size q=0; q<size; q++) {
            diagonal_[q] = s[q][q];
            eigenVectors_[q][q] = 1.0;
        }
        Matrix ss = s;

        std::vector<Real> tmpDiag(diagonal_.begin(), diagonal_.end());
        std::vector<Real> tmpAccumulate(size, 0.0);
        Real threshold, epsPrec = 1e-15;
        bool keeplooping = true;
        Size maxIterations = 100, ite = 1;
        do {
            //main loop
            Real sum = 0;
            for (Size a=0; a<size-1; a++) {
                for (Size b=a+1; b<size; b++) {
                    sum += QL_FABS(ss[a][b]);
                }
            }

            if (sum==0) {
                keeplooping = false;
            } else {
                /* To speed up computation a threshold is introduced to
                   make sure it is worthy to perform the Jacobi rotation
                */
                if (ite<5) threshold = 0.2*sum/(size*size);
                else       threshold = 0.0;

                Size j, k, l;
                for (j=0; j<size-1; j++) {
                    for (k=j+1; k<size; k++) {
                        Real sine, rho, cosin, heig, tang, beta;
                        Real smll = QL_FABS(ss[j][k]);
                        if(ite> 5 &&
                           smll<epsPrec*QL_FABS(diagonal_[j]) &&
                           smll<epsPrec*QL_FABS(diagonal_[k])) {
                                ss[j][k] = 0;
                        } else if (QL_FABS(ss[j][k])>threshold) {
                            heig = diagonal_[k]-diagonal_[j];
                            if (smll<epsPrec*QL_FABS(heig)) {
                                tang = ss[j][k]/heig;
                            } else {
                                beta = 0.5*heig/ss[j][k];
                                tang = 1.0/(QL_FABS(beta)+
                                    QL_SQRT(1+beta*beta));
                                if (beta<0)
                                    tang = -tang;
                            }
                            cosin = 1/QL_SQRT(1+tang*tang);
                            sine = tang*cosin;
                            rho = sine/(1+cosin);
                            heig = tang*ss[j][k];
                            tmpAccumulate[j] -= heig;
                            tmpAccumulate[k] += heig;
                            diagonal_[j] -= heig;
                            diagonal_[k] += heig;
                            ss[j][k] = 0.0;
                            for (l=0; l+1<=j; l++)
                                jacobiRotate_(ss, rho, sine, l, j, l, k);
                            for (l=j+1; l<=k-1; l++)
                                jacobiRotate_(ss, rho, sine, j, l, l, k);
                            for (l=k+1; l<size; l++)
                                jacobiRotate_(ss, rho, sine, j, l, k, l);
                            for (l=0;   l<size; l++)
                                jacobiRotate_(eigenVectors_,
                                                  rho, sine, l, j, l, k);
                        }
                    }
                }
                for (k=0; k<size; k++) {
                    tmpDiag[k] += tmpAccumulate[k];
                    diagonal_[k] = tmpDiag[k];
                    tmpAccumulate[k] = 0.0;
                }
            }
        } while (++ite<=maxIterations && keeplooping);

        QL_REQUIRE(ite<=maxIterations,
                   "Too many iterations reached");


        // sort (eigenvalues, eigenvectors)
        std::vector<std::pair<Real, std::vector<Real> > > temp(size);
        std::vector<Real> eigenVector(size);
        Size row, col;
        for (col=0; col<size; col++) {
            #if defined(QL_PATCH_BORLAND)
            for (row=0; row<size; row++) {
                eigenVector[row] = eigenVectors_[row][col];
            }
            #else
            // doesn't work for Borland
            std::copy(eigenVectors_.column_begin(col),
                      eigenVectors_.column_end(col), eigenVector.begin());
            #endif
            temp[col] = std::make_pair<Real, std::vector<Real> >(
                diagonal_[col], eigenVector);
        }
        std::sort(temp.begin(), temp.end(),
            std::greater<std::pair<Real, std::vector<Real> > >());
        Real maxEv = temp[0].first;
        for (col=0; col<size; col++) {
            // check for round-off errors
            diagonal_[col] = 
                (QL_FABS(temp[col].first/maxEv)<1e-16 ? 0.0 : temp[col].first);
            Real sign = 1.0;
            if (temp[col].second[0]<0.0)
                sign = -1.0;
            // doesn't work at all :(
            // std::copy(eigenVector.begin(), eigenVector.end(),
            //    eigenVectors_.column_begin(col));
            for (row=0; row<size; row++) {
                eigenVectors_[row][col] = sign * temp[col].second[row];
            }
        }
    }

}
