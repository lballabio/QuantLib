/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/math/matrixutilities/factorreduction.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>
#include <vector>

namespace QuantLib {

    std::vector<Real> factorReduction(Matrix mtrx,
                                      Size maxIters) {
        static Real tolerance = 1.e-6;

        QL_REQUIRE(mtrx.rows() == mtrx.columns(),
                   "Input matrix is not square");

        const Size n = mtrx.columns();

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        // check symmetry
        for (Size iRow=0; iRow<mtrx.rows(); iRow++)
            for (Size iCol=0; iCol<iRow; iCol++)
                QL_REQUIRE(mtrx[iRow][iCol] == mtrx[iCol][iRow],
                           "input matrix is not symmetric");
        QL_REQUIRE(*std::max_element(mtrx.begin(), mtrx.end()) <=1 &&
            *std::min_element(mtrx.begin(), mtrx.end()) >=-1,
                    "input matrix data is not correlation data");
        #endif

        // Initial guess value
        std::vector<Real> previousCorrels(n, 0.);
        for(Size iCol=0; iCol<n; iCol++) {
            for(Size iRow=0; iRow<n; iRow++)
                previousCorrels[iCol] +=
                    mtrx[iRow][iCol] * mtrx[iRow][iCol];
            previousCorrels[iCol] =
                std::sqrt((previousCorrels[iCol]-1.)/(n-1.));
        }

        // iterative solution
        Size iteration = 0;
        Real distance;
        do {
            // patch Matrix diagonal
            for(Size iCol=0; iCol<n; iCol++)
                mtrx[iCol][iCol] =
                    previousCorrels[iCol];
            // compute eigenvector decomposition
            SymmetricSchurDecomposition ssDec(mtrx);
            //const Matrix& eigenVect = ssDec.eigenvectors();
            const Array&  eigenVals = ssDec.eigenvalues();
            Array::const_iterator itMaxEval =
                std::max_element(eigenVals.begin(), eigenVals.end());
            // We do not need the max value, only the position of the
            //   corresponding eigenvector
            Size iMax = std::distance(eigenVals.begin(), itMaxEval);
            std::vector<Real> newCorrels, distances;
            for(Size iCol=0; iCol<n; iCol++) {
                Real thisCorrel = mtrx[iMax][iCol];
                newCorrels.push_back(thisCorrel);
                // strictly is:
                // abs(\sqrt{\rho}- \sqrt{\rho_{old}})/\sqrt{\rho_{old}}
                distances.push_back(
                    std::abs(thisCorrel - previousCorrels[iCol])/
                        previousCorrels[iCol]);
            }
            previousCorrels = newCorrels;
            distance = *std::max_element(distances.begin(), distances.end());
        }while(distance > tolerance && ++iteration <= maxIters );

        // test it did not go up to the max iteration and the matrix can
        //   be reduced to one factor.
        QL_REQUIRE(iteration < maxIters,
                   "convergence not reached after " <<
                   iteration << " iterations");

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(*std::max_element(previousCorrels.begin(),
                                     previousCorrels.end()) <=1 &&
                   *std::min_element(previousCorrels.begin(),
                                     previousCorrels.end()) >=-1,
                "matrix can not be decomposed to a single factor dependence");
        #endif

        return previousCorrels;
    }

}
