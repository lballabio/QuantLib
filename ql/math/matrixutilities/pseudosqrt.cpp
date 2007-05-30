/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Yiping Chen
 Copyright (C) 2007 Neil Firth

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

#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/optimization/conjugategradient.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace QuantLib {

    namespace {

        void checkSymmetry(const Matrix& matrix) {
            Size size = matrix.rows();
            QL_REQUIRE(size == matrix.columns(),
                       "non square matrix: " << size << " rows, " <<
                       matrix.columns() << " columns");
            for (Size i=0; i<size; ++i)
                for (Size j=0; j<i; ++j)
                    QL_REQUIRE(close(matrix[i][j], matrix[j][i]),
                               "non symmetric matrix: " <<
                               "[" << i << "][" << j << "]=" << matrix[i][j] <<
                               ", [" << j << "][" << i << "]=" << matrix[j][i]);
        }

        void normalizePseudoRoot(const Matrix& matrix,
                                 Matrix& pseudo) {
            Size size = matrix.rows();
            QL_REQUIRE(size == pseudo.rows(),
                       "matrix/pseudo mismatch: matrix rows are " << size <<
                       " while pseudo rows are " << pseudo.columns());
            Size pseudoCols = pseudo.columns();

            // row normalization
            for (Size i=0; i<size; ++i) {
                Real norm = 0.0;
                for (Size j=0; j<pseudoCols; ++j)
                    norm += pseudo[i][j]*pseudo[i][j];
                if (norm>0.0) {
                    Real normAdj = std::sqrt(matrix[i][i]/norm);
                    for (Size j=0; j<pseudoCols; ++j)
                        pseudo[i][j] *= normAdj;
                }
            }


        }

        //cost function for hypersphere and lower-diagonal algorithm
        class HypersphereCostFunction : public CostFunction {
          private:
            Size size_;
            bool lowerDiagonal_;
            Matrix targetMatrix_;
            Array targetVariance_;
            mutable Matrix currentRoot_, tempMatrix_, currentMatrix_;
          public:
            HypersphereCostFunction(const Matrix& targetMatrix,
                                    const Array& targetVariance,
                                    bool lowerDiagonal)
            : size_(targetMatrix.rows()), lowerDiagonal_(lowerDiagonal),
              targetMatrix_(targetMatrix), targetVariance_(targetVariance),
              currentRoot_(size_, size_), tempMatrix_(size_, size_),
              currentMatrix_(size_, size_) {}
            Disposable<Array> values(const Array&) const {
                QL_FAIL("values method not implemented");
            }
            Real value(const Array& x) const {
                Size i,j,k;
                std::fill(currentRoot_.begin(), currentRoot_.end(), 1.0);
                if (lowerDiagonal_) {
                    for (i=0; i<size_; i++) {
                        for (k=0; k<size_; k++) {
                            if (k>i) {
                                currentRoot_[i][k]=0;
                            } else {
                                for (j=0; j<=k; j++) {
                                    if (j == k && k!=i)
                                        currentRoot_[i][k] *=
                                            std::cos(x[i*(i-1)/2+j]);
                                    else if (j!=i)
                                        currentRoot_[i][k] *=
                                            std::sin(x[i*(i-1)/2+j]);
                                }
                            }
                        }
                    }
                } else {
                    for (i=0; i<size_; i++) {
                        for (k=0; k<size_; k++) {
                            for (j=0; j<=k; j++) {
                                if (j == k && k!=size_-1)
                                    currentRoot_[i][k] *=
                                        std::cos(x[j*size_+i]);
                                else if (j!=size_-1)
                                    currentRoot_[i][k] *=
                                        std::sin(x[j*size_+i]);
                            }
                        }
                    }
                }
                Real temp, error=0;
                tempMatrix_ = transpose(currentRoot_);
                currentMatrix_ = currentRoot_ * tempMatrix_;
                for (i=0;i<size_;i++) {
                    for (j=0;j<size_;j++) {
                        temp = currentMatrix_[i][j]*targetVariance_[i]
                          *targetVariance_[j]-targetMatrix_[i][j];
                        error += temp*temp;
                    }
                }
                return error;
            }
        };

        // Optimization function for hypersphere and lower-diagonal algorithm
        const Disposable <Matrix> hypersphereOptimize(
                                                const Matrix& targetMatrix,
                                                const Matrix& currentRoot,
                                                const bool lowerDiagonal) {
            Size i,j,k,size = targetMatrix.rows();
            Matrix result(currentRoot);
            Array variance(size, 0);
            for (i=0; i<size; i++){
                variance[i]=std::sqrt(targetMatrix[i][i]);
            }
            if (lowerDiagonal) {
                Matrix approxMatrix(result*transpose(result));
                result = CholeskyDecomposition(approxMatrix, true);
                for (i=0; i<size; i++) {
                    for (j=0; j<size; j++) {
                        result[i][j]/=std::sqrt(approxMatrix[i][i]);
                    }
                }
            } else {
                for (i=0; i<size; i++) {
                    for (j=0; j<size; j++) {
                        result[i][j]/=variance[i];
                    }
                }
            }

            ConjugateGradient optimize;
            EndCriteria endCriteria(100, 10, 1e-8, 1e-8, 1e-8);
            HypersphereCostFunction costFunction(targetMatrix, variance,
                                                 lowerDiagonal);
            NoConstraint constraint;

            // hypersphere vector optimization

            if (lowerDiagonal) {
                Array theta(size * (size-1)/2);
                const Real eps=1e-16;
                for (i=1; i<size; i++) {
                    for (j=0; j<i; j++) {
                        theta[i*(i-1)/2+j]=result[i][j];
                        if (theta[i*(i-1)/2+j]>1-eps)
                            theta[i*(i-1)/2+j]=1-eps;
                        if (theta[i*(i-1)/2+j]<-1+eps)
                            theta[i*(i-1)/2+j]=-1+eps;
                        for (k=0; k<j; k++) {
                            theta[i*(i-1)/2+j] /= std::sin(theta[i*(i-1)/2+k]);
                            if (theta[i*(i-1)/2+j]>1-eps)
                                theta[i*(i-1)/2+j]=1-eps;
                            if (theta[i*(i-1)/2+j]<-1+eps)
                                theta[i*(i-1)/2+j]=-1+eps;
                        }
                        theta[i*(i-1)/2+j] = std::acos(theta[i*(i-1)/2+j]);
                        if (j==i-1) {
                            if (result[i][i]<0)
                                theta[i*(i-1)/2+j]=-theta[i*(i-1)/2+j];
                        }
                    }
                }
                Problem p(costFunction, constraint, theta);
                optimize.minimize(p, endCriteria);
                theta = p.currentValue();
                std::fill(result.begin(),result.end(),1.0);
                for (i=0; i<size; i++) {
                    for (k=0; k<size; k++) {
                        if (k>i) {
                            result[i][k]=0;
                        } else {
                            for (j=0; j<=k; j++) {
                                if (j == k && k!=i)
                                    result[i][k] *=
                                        std::cos(theta[i*(i-1)/2+j]);
                                else if (j!=i)
                                    result[i][k] *=
                                        std::sin(theta[i*(i-1)/2+j]);
                            }
                        }
                    }
                }
            } else {
                Array theta(size * (size-1));
                const Real eps=1e-16;
                for (i=0; i<size; i++) {
                    for (j=0; j<size-1; j++) {
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
                Problem p(costFunction, constraint, theta);
                optimize.minimize(p, endCriteria);
                theta=p.currentValue();
                std::fill(result.begin(),result.end(),1.0);
                for (i=0; i<size; i++) {
                    for (k=0; k<size; k++) {
                        for (j=0; j<=k; j++) {
                            if (j == k && k!=size-1)
                                result[i][k] *= std::cos(theta[j*size+i]);
                            else if (j!=size-1)
                                result[i][k] *= std::sin(theta[j*size+i]);
                        }
                    }
                }
            }

            for (i=0; i<size; i++) {
                for (j=0; j<size; j++) {
                    result[i][j]*=variance[i];
                }
            }
            return result;
        }

        // Matrix infinity norm. See Golub and van Loan (2.3.10) or
        // <http://en.wikipedia.org/wiki/Matrix_norm>
        Real normInf(const Matrix& M) {
            Size rows = M.rows();
            Size cols = M.columns();
            Real norm = 0.0;
            for (Size i=0; i<rows; ++i) {
                Real colSum = 0.0;
                for (Size j=0; j<cols; ++j)
                    colSum += std::fabs(M[i][j]);
                norm = std::max(norm, colSum);
            }
            return norm;
        }

        // Take a matrix and make all the diagonal entries 1.
        const Disposable <Matrix>
        projectToUnitDiagonalMatrix(const Matrix& M) {
            Size size = M.rows();
            QL_REQUIRE(size == M.columns(),
                       "matrix not square");

            Matrix result(M);
            for (Size i=0; i<size; ++i)
                result[i][i] = 1.0;

            return result;
        }

        // Take a matrix and make all the eigenvalues non-negative
        const Disposable <Matrix>
        projectToPositiveSemidefiniteMatrix(Matrix& M) {
            Size size = M.rows();
            QL_REQUIRE(size == M.columns(),
                       "matrix not square");

            Matrix diagonal(size, size, 0.0);
            SymmetricSchurDecomposition jd(M);
            for (Size i=0; i<size; ++i)
                diagonal[i][i] = std::max<Real>(jd.eigenvalues()[i], 0.0);

            Matrix result =
                jd.eigenvectors()*diagonal*transpose(jd.eigenvectors());
            return result;
        }

        // implementation of the Higham algorithm to find the nearest
        // correlation matrix.
        const Disposable <Matrix>
        highamImplementation(const Matrix& A,
                             const Size maxIterations,
                             const Real& tolerance) {

            Size size = A.rows();
            Matrix R, Y(A), X(A), deltaS(size, size, 0.0);

            Matrix lastX(X);
            Matrix lastY(Y);

            for (Size i=0; i<maxIterations; ++i) {
                R = Y - deltaS;
                X = projectToPositiveSemidefiniteMatrix(R);
                deltaS = X - R;
                Y = projectToUnitDiagonalMatrix(X);

                // convergence test
                if (std::max(normInf(X-lastX)/normInf(X),
                        std::max(normInf(Y-lastY)/normInf(Y),
                                normInf(Y-X)/normInf(Y)))
                        <= tolerance)
                {
                    break;
                }
                lastX = X;
                lastY = Y;
            }

            // ensure we return a symmetric matrix
            for (Size i=0; i<size; ++i)
                for (Size j=0; j<i; ++j)
                    Y[i][j] = Y[j][i];

            return Y;
        }

    }


    const Disposable<Matrix> pseudoSqrt(const Matrix& matrix,
                                        SalvagingAlgorithm::Type sa) {
        Size size = matrix.rows();

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        checkSymmetry(matrix);
        #else
        QL_REQUIRE(size == matrix.columns(),
                   "non square matrix: " << size << " rows, " <<
                   matrix.columns() << " columns");
        #endif

        // spectral (a.k.a Principal Component) analysis
        SymmetricSchurDecomposition jd(matrix);
        Matrix diagonal(size, size, 0.0);

        // salvaging algorithm
        Matrix result(size, size);
        bool negative;
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
            for (Size i=0; i<size; i++)
                diagonal[i][i] =
                    std::sqrt(std::max<Real>(jd.eigenvalues()[i], 0.0));

            result = jd.eigenvectors() * diagonal;
            normalizePseudoRoot(matrix, result);
            break;
          case SalvagingAlgorithm::Hypersphere:
            // negative eigenvalues set to zero
            negative=false;
            for (Size i=0; i<size; ++i){
                diagonal[i][i] =
                    std::sqrt(std::max<Real>(jd.eigenvalues()[i], 0.0));
                if (jd.eigenvalues()[i]<0.0) negative=true;
            }
            result = jd.eigenvectors() * diagonal;
            normalizePseudoRoot(matrix, result);

            if (negative)
                result = hypersphereOptimize(matrix, result, false);
            break;
          case SalvagingAlgorithm::LowerDiagonal:
            // negative eigenvalues set to zero
            negative=false;
            for (Size i=0; i<size; ++i){
                diagonal[i][i] =
                    std::sqrt(std::max<Real>(jd.eigenvalues()[i], 0.0));
                if (jd.eigenvalues()[i]<0.0) negative=true;
            }
            result = jd.eigenvectors() * diagonal;

            normalizePseudoRoot(matrix, result);

            if (negative)
                result = hypersphereOptimize(matrix, result, true);
            break;
          case SalvagingAlgorithm::Higham: {
              int maxIterations = 40;
              Real tol = 1e-6;
              result = highamImplementation(matrix, maxIterations, tol);
              result = CholeskyDecomposition(result, true);
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

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        checkSymmetry(matrix);
        #else
        QL_REQUIRE(size == matrix.columns(),
                   "non square matrix: " << size << " rows, " <<
                   matrix.columns() << " columns");
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
            for (Size i=0; i<size; ++i)
                eigenValues[i] = std::max<Real>(eigenValues[i], 0.0);
            break;
          case SalvagingAlgorithm::Higham:
              {
                  int maxIterations = 40;
                  Real tolerance = 1e-6;
                  Matrix adjustedMatrix = highamImplementation(matrix, maxIterations, tolerance);
                  jd = SymmetricSchurDecomposition(adjustedMatrix);
                  eigenValues = jd.eigenvalues();
              }
              break;
          default:
            QL_FAIL("unknown or invalid salvaging algorithm");
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
        for (Size i=1; components<enough && i<size; ++i) {
            components += eigenValues[i];
            retainedFactors++;
        }
        // output is granted to have a rank<=maxRank
        retainedFactors=std::min(retainedFactors, maxRank);

        Matrix diagonal(size, retainedFactors, 0.0);
        for (Size i=0; i<retainedFactors; ++i)
            diagonal[i][i] = std::sqrt(eigenValues[i]);
        Matrix result = jd.eigenvectors() * diagonal;

        normalizePseudoRoot(matrix, result);
        return result;
    }

}
