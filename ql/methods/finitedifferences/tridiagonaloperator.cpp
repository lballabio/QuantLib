/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2011 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    TridiagonalOperator::TridiagonalOperator(Size size) {
        if (size>=2) {
            n_ = size;
            diagonal_      = Array(size);
            lowerDiagonal_ = Array(size-1);
            upperDiagonal_ = Array(size-1);
            temp_          = Array(size);
        } else if (size==0) {
            n_ = 0;
            diagonal_      = Array(0);
            lowerDiagonal_ = Array(0);
            upperDiagonal_ = Array(0);
            temp_          = Array(0);
        } else {
            QL_FAIL("invalid size (" << size << ") for tridiagonal operator "
                    "(must be null or >= 2)");
        }
    }

    TridiagonalOperator::TridiagonalOperator(const Array& low,
                                             const Array& mid,
                                             const Array& high)
    : n_(mid.size()),
      diagonal_(mid), lowerDiagonal_(low), upperDiagonal_(high), temp_(n_) {
        QL_REQUIRE(low.size() == n_-1,
                   "low diagonal vector of size " << low.size() <<
                   " instead of " << n_-1);
        QL_REQUIRE(high.size() == n_-1,
                   "high diagonal vector of size " << high.size() <<
                   " instead of " << n_-1);
    }

    Array TridiagonalOperator::applyTo(const Array& v) const {
        QL_REQUIRE(n_!=0,
                   "uninitialized TridiagonalOperator");
        QL_REQUIRE(v.size()==n_,
                   "vector of the wrong size " << v.size() <<
                   " instead of " << n_);
        Array result(n_);
        std::transform(diagonal_.begin(), diagonal_.end(),
                       v.begin(),
                       result.begin(),
                       std::multiplies<>());

        // matricial product
        result[0] += upperDiagonal_[0]*v[1];
        for (Size j=1; j<=n_-2; j++)
            result[j] += lowerDiagonal_[j-1]*v[j-1]+
                upperDiagonal_[j]*v[j+1];
        result[n_-1] += lowerDiagonal_[n_-2]*v[n_-2];

        return result;
    }

    Array TridiagonalOperator::solveFor(const Array& rhs) const  {
        Array result(rhs.size());
        solveFor(rhs, result);
        return result;
    }

    void TridiagonalOperator::solveFor(const Array& rhs,
                                       Array& result) const  {

        QL_REQUIRE(n_!=0,
                   "uninitialized TridiagonalOperator");
        QL_REQUIRE(rhs.size()==n_,
                   "rhs vector of size " << rhs.size() <<
                   " instead of " << n_);

        Real bet = diagonal_[0];
        QL_REQUIRE(!close(bet, 0.0),
                   "diagonal's first element (" << bet <<
                   ") cannot be close to zero");
        result[0] = rhs[0]/bet;
        for (Size j=1; j<=n_-1; ++j) {
            temp_[j] = upperDiagonal_[j-1]/bet;
            bet = diagonal_[j]-lowerDiagonal_[j-1]*temp_[j];
            QL_ENSURE(!close(bet, 0.0), "division by zero");
            result[j] = (rhs[j] - lowerDiagonal_[j-1]*result[j-1])/bet;
        }
        // cannot be j>=0 with Size j
        for (Size j=n_-2; j>0; --j)
            result[j] -= temp_[j+1]*result[j+1];
        result[0] -= temp_[1]*result[1];
    }

    Array TridiagonalOperator::SOR(const Array& rhs,
                                   Real tol) const {
        QL_REQUIRE(n_!=0,
                   "uninitialized TridiagonalOperator");
        QL_REQUIRE(rhs.size()==n_,
                   "rhs vector of size " << rhs.size() <<
                   " instead of " << n_);

        // initial guess
        Array result = rhs;

        // solve tridiagonal system with SOR technique
        Real omega = 1.5;
        Real err = 2.0*tol;
        Real temp;
        for (Size sorIteration=0; err>tol ; ++sorIteration) {
            QL_REQUIRE(sorIteration<100000,
                       "tolerance (" << tol << ") not reached in " <<
                       sorIteration << " iterations. " <<
                       "The error still is " << err);

            temp = omega * (rhs[0]     -
                            upperDiagonal_[0]   * result[1]-
                            diagonal_[0]        * result[0])/diagonal_[0];
            err = temp*temp;
            result[0] += temp;
            Size i;
            for (i=1; i<n_-1 ; ++i) {
                temp = omega *(rhs[i]     -
                               upperDiagonal_[i]   * result[i+1]-
                               diagonal_[i]        * result[i] -
                               lowerDiagonal_[i-1] * result[i-1])/diagonal_[i];
                err += temp * temp;
                result[i] += temp;
            }

            temp = omega * (rhs[i]     -
                            diagonal_[i]        * result[i] -
                            lowerDiagonal_[i-1] * result[i-1])/diagonal_[i];
            err += temp*temp;
            result[i] += temp;
        }
        return result;
    }

    TridiagonalOperator TridiagonalOperator::identity(Size size) {
        return TridiagonalOperator(Array(size-1, 0.0),     // lower diagonal
                                   Array(size,   1.0),     // diagonal
                                   Array(size-1, 0.0));    // upper diagonal
    }

}
