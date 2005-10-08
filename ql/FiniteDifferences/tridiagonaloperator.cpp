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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    TridiagonalOperator::TridiagonalOperator(Size size) {
        if (size>=3) {
            diagonal_      = Array(size);
            lowerDiagonal_ = Array(size-1);
            upperDiagonal_ = Array(size-1);
        } else if (size==0) {
            diagonal_      = Array(0);
            lowerDiagonal_ = Array(0);
            upperDiagonal_ = Array(0);
        } else {
            QL_FAIL("invalid size for tridiagonal operator "
                    "(must be null or >= 3)");
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

    Disposable<TridiagonalOperator>
    TridiagonalOperator::identity(Size size) {
        TridiagonalOperator I(Array(size-1, 0.0),     // lower diagonal
                              Array(size,   1.0),     // diagonal
                              Array(size-1, 0.0));    // upper diagonal
        return I;
    }

}
