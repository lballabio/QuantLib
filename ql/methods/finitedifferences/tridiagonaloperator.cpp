/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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
            diagonal_      = Array(size);
            lowerDiagonal_ = Array(size-1);
            upperDiagonal_ = Array(size-1);
        } else if (size==0) {
            diagonal_      = Array(0);
            lowerDiagonal_ = Array(0);
            upperDiagonal_ = Array(0);
        } else {
            QL_FAIL("invalid size (" << size << ") for tridiagonal operator "
                    "(must be null or >= 2)");
        }
    }

    TridiagonalOperator::TridiagonalOperator(const Array& low,
                                             const Array& mid,
                                             const Array& high)
    : diagonal_(mid), lowerDiagonal_(low), upperDiagonal_(high) {
        QL_REQUIRE(low.size() == mid.size()-1,
                   "wrong size for lower diagonal vector");
        QL_REQUIRE(high.size() == mid.size()-1,
                   "wrong size for upper diagonal vector");
    }

    Disposable<Array> TridiagonalOperator::applyTo(const Array& v) const {
        QL_REQUIRE(v.size()==size(),
                   "vector of the wrong size (" << v.size()
                   << "instead of " << size() << ")"  );
        Array result(size());
        std::transform(diagonal_.begin(), diagonal_.end(),
                       v.begin(),
                       result.begin(),
                       std::multiplies<Real>());

        // matricial product
        result[0] += upperDiagonal_[0]*v[1];
        for (Size j=1; j<=size()-2; j++)
            result[j] += lowerDiagonal_[j-1]*v[j-1]+
                upperDiagonal_[j]*v[j+1];
        result[size()-1] += lowerDiagonal_[size()-2]*v[size()-2];

        return result;
    }

    Disposable<Array> TridiagonalOperator::solveFor(const Array& rhs) const  {
        QL_REQUIRE(rhs.size()==size(), "rhs has the wrong size");

        Array result(size()), tmp(size());

        Real bet=diagonal_[0];
        QL_REQUIRE(bet != 0.0, "division by zero");
        result[0] = rhs[0]/bet;
        Size j;
        for (j=1; j<=size()-1; j++){
            tmp[j]=upperDiagonal_[j-1]/bet;
            bet=diagonal_[j]-lowerDiagonal_[j-1]*tmp[j];
            QL_ENSURE(bet != 0.0, "division by zero");
            result[j] = (rhs[j]-lowerDiagonal_[j-1]*result[j-1])/bet;
        }
        // cannot be j>=0 with Size j
        for (j=size()-2; j>0; --j)
            result[j] -= tmp[j+1]*result[j+1];
        result[0] -= tmp[1]*result[1];
        return result;
    }

    Disposable<Array> TridiagonalOperator::SOR(const Array& rhs,
                                               Real tol) const {
        QL_REQUIRE(rhs.size()==size(), "rhs has the wrong size");

        // initial guess
        Array result = rhs;

        // solve tridiagonal system with SOR technique
        Size sorIteration, i;
        Real omega = 1.5;
        Real err = 2.0*tol;
        Real temp;
        for (sorIteration=0; err>tol ; sorIteration++) {
            QL_REQUIRE(sorIteration<100000,
                       "tolerance (" << tol << ") not reached in "
                       << sorIteration << " iterations. "
                       << "The error still is " << err);

            temp = omega * (rhs[0]     -
                            upperDiagonal_[0]   * result[1]-
                            diagonal_[0]        * result[0])/diagonal_[0];
            err = temp*temp;
            result[0] += temp;

            for (i=1; i<size()-1 ; i++) {
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

    Disposable<TridiagonalOperator>
    TridiagonalOperator::identity(Size size) {
        TridiagonalOperator I(Array(size-1, 0.0),     // lower diagonal
                              Array(size,   1.0),     // diagonal
                              Array(size-1, 0.0));    // upper diagonal
        return I;
    }

}
