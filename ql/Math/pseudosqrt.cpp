/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2006 Yiping Chen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Math/pseudosqrt.hpp>
#include <ql/Math/choleskydecomposition.hpp>
#include <ql/Math/symmetricschurdecomposition.hpp>
#include <ql/Optimization/conjugategradient.hpp>

namespace QuantLib {

    namespace {

        class HypersphereCostFunction : public CostFunction {
          private:
            Matrix matrix_;
            Size size_;
          public:
            void setMatrix(const Matrix& matrix){
                matrix_=matrix;
            }
            void setSize(const Size& size){
                size_=size;
            }
            Real value(const Array& x) const{
                Size i,j,k;
                Matrix result(size_,size_,1.0);
                Matrix resultTranspose(size_,size_);
                Matrix resultProduct(size_,size_);
                for (i=0;i<size_;i++) {
                    for (k=0;k<size_;k++) {
                        for (j=0;j<=k;j++) {
                            if (j == k && k!=size_-1)
                                result[i][k] *= std::cos(x[j*size_+i]);
                            else
                                if (j!=size_-1)
                                    result[i][k] *= std::sin(x[j*size_+i]);
                        }
                    }
                }
                Real value=0;
                resultTranspose=transpose(result);
                resultProduct=result * resultTranspose;
                for (i=0;i<size_;i++) {
                    for (j=0;j<size_;j++) {
                        value+=pow(resultProduct[i][j]-matrix_[i][j],2.);
                    }
                }
                return value;
            }
        };

    }


    const Disposable<Matrix> pseudoSqrt(const Matrix& matrix,
                                        SalvagingAlgorithm::Type sa) {
        Size size = matrix.rows();
        QL_REQUIRE(size == matrix.columns(),
                   "matrix not square");
        Size i, j, k;
        bool negative;
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<size; i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(matrix[i][j] == matrix[j][i],
                           "matrix not symmetric");
        #endif

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Matrix diagonal(size, size, 0.0);

        // salvaging algorithm
        Matrix result(size, size);
        switch (sa) {
          case SalvagingAlgorithm::None:
            // eigenvalues are sorted in decreasing order
            QL_REQUIRE(jd.eigenvalues()[size-1]>=-1e-16,
                       "negative eigenvalue(s) ("
                       << std::scientific << jd.eigenvalues()[size-1]
                       << ")");
            result = CholeskyDecomposition(matrix, true);
            break;
          case SalvagingAlgorithm::Spectral:
            // negative eigenvalues set to zero
            for (i=0; i<size; i++)
                diagonal[i][i] =
                    std::sqrt(std::max<Real>(jd.eigenvalues()[i], 0.0));

            result = jd.eigenvectors() * diagonal;
            // row normalization
            for (i = 0; i < size;i++) {
                Real norm = 0.0;
                for (j = 0; j < size; j++)
                    norm += result[i][j]*result[i][j];
                norm = std::sqrt(matrix[i][i]/norm);
                for (j = 0; j < size; j++)
                    result[i][j] *= norm;
            }
            break;
          case SalvagingAlgorithm::Hypersphere:
            // negative eigenvalues set to zero
            negative=false;
            for (i=0; i<size; i++){
                diagonal[i][i] =
                    std::sqrt(std::max<Real>(jd.eigenvalues()[i], 0.0));
                if (jd.eigenvalues()[i]<0.0) negative=true;
            }
            result = jd.eigenvectors() * diagonal;
            // row normalization
            for (i = 0; i < size; i++) {
                Real norm = 0.0;
                for (j = 0; j < size; j++)
                    norm += result[i][j]*result[i][j];
                norm = std::sqrt(matrix[i][i]/norm);
                for (j = 0; j < size; j++)
                    result[i][j] *= norm;
            }

            if (negative) {
                // hypersphere vector initialization
                Array theta(size * (size-1), 1., 1.);
                const Real eps=1e-16;
                for (i=0;i<size;i++) {
                    for (j=0;j<size-1;j++) {
                        theta[j*size+i]=result[i][j];
                        if (theta[j*size+i]>1-eps)
                            theta[j*size+i]=1-eps;
                        if (theta[j*size+i]<-1+eps)
                            theta[j*size+i]=-1+eps;
                        for (k=0;k<j;k++) {
                            theta[j*size+i] /= std::sin(theta[k*size+i]);
                            if (theta[j*size+i]>1-eps)
                                theta[j*size+i]=1-eps;
                            if (theta[j*size+i]<-1+eps)
                                theta[j*size+i]=-1+eps;
                        }
                        theta[j*size+i] = std::acos(theta[j*size+i]);
                        if (j==size-2) {
                            if (result[i][j+1]<0)
                                theta[j*size+i]=-theta[j*size+i];
                        }
                    }
                }

                // Optimization
                EndCriteria endCriteria(100, 1e-8);
                ConjugateGradient optimize;
                optimize.setInitialValue(theta);
                optimize.setEndCriteria(endCriteria);
                HypersphereCostFunction costFunction;
                costFunction.setMatrix(matrix);
                costFunction.setSize(size);
                NoConstraint constraint;
                Problem p(costFunction, constraint, optimize);
                p.minimize();
                theta=p.minimumValue();

                // Conversion from hypersphere vector to matrix
                std::fill(result.begin(),result.end(),1.0);
                for (i=0;i<size;i++) {
                    for (k=0;k<size;k++) {
                        for (j=0;j<=k;j++) {
                            if (j == k && k!=size-1)
                                result[i][k] *= std::cos(theta[j*size+i]);
                            else
                                if (j!=size-1)
                                    result[i][k] *= std::sin(theta[j*size+i]);
                        }
                    }
                }
            }
            break;
          default:
            QL_FAIL("unknown salvaging algorithm");
        }

        return result;
    }


    const Disposable<Matrix> rankReducedSqrt(const Matrix& matrix,
                                             Size maxRank,
                                             Real componentRetainedPercentage,
                                             SalvagingAlgorithm::Type sa) {
        Size size = matrix.rows();
        QL_REQUIRE(size == matrix.columns(),
                   "matrix not square");
        Size i, j;
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        for (i=0; i<size; i++)
            for (j=0; j<i; j++)
                QL_REQUIRE(matrix[i][j] == matrix[j][i],
                           "matrix not symmetric");
        #endif

        QL_REQUIRE(componentRetainedPercentage>0.0,
                   "no eigenvalues retained");

        QL_REQUIRE(componentRetainedPercentage<=1.0,
                   "percentage to be retained > 100%");

        QL_REQUIRE(maxRank>=1,
                   "max rank required < 1");

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Array eigenValues = jd.eigenvalues();

        // salvaging algorithm
        switch (sa) {
          case SalvagingAlgorithm::None:
            // eigenvalues are sorted in decreasing order
            QL_REQUIRE(eigenValues[size-1]>=-1e-16,
                       "negative eigenvalue(s) ("
                       << std::scientific << eigenValues[size-1]
                       << ")");
            break;
          case SalvagingAlgorithm::Spectral:
            // negative eigenvalues set to zero
            for (i=0; i<size; i++)
                eigenValues[i] = std::max<Real>(eigenValues[i], 0.0);
            break;
          default:
            QL_FAIL("unknown salvaging algorithm");
        }

        // factor reduction
        Real enough = componentRetainedPercentage *
                      std::accumulate(eigenValues.begin(),
                                      eigenValues.end(), 0.0);
        if (componentRetainedPercentage == 1.0) {
            // numerical glitches might cause some factors to be discarded
            enough *= 1.1;
        }
        // retain at least one factor
        Real components = eigenValues[0];
        Size retainedFactors = 1;
        for (i=1; components<enough && i<size; i++) {
            components += eigenValues[i];
            retainedFactors++;
        }
        // output is granted to have a rank<=maxRank
        retainedFactors=std::min(retainedFactors, maxRank);

        Matrix diagonal(size, retainedFactors, 0.0);
        for (i=0; i<retainedFactors; i++)
            diagonal[i][i] = std::sqrt(eigenValues[i]);
        Matrix result = jd.eigenvectors() * diagonal;

        // row normalization
        for (i = 0; i < size;i++) {
            Real norm = 0.0;
            for (j = 0; j < retainedFactors; j++)
                norm += result[i][j]*result[i][j];
            if (norm>0.0) {
                norm = std::sqrt(matrix[i][i]/norm);
                for(j = 0; j < retainedFactors; j++)
                    result[i][j] *= norm;
            }
        }

        return result;
    }

}

