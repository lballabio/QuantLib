

/*
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file tridiagonaloperator.cpp
    \brief tridiagonal operator

    \fullpath
    ql/FiniteDifferences/%tridiagonaloperator.cpp
*/

// $Id$

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/dataformatters.hpp>
#include <iostream>

namespace QuantLib {

    namespace FiniteDifferences {

        TridiagonalOperator::TridiagonalOperator(
            Size size) {

            if (size>=3) {
                diagonal_      = Array(size);
                belowDiagonal_ = Array(size-1);
                aboveDiagonal_ = Array(size-1);
            } else if (size==0) {
                diagonal_      = Array(0);
                belowDiagonal_ = Array(0);
                aboveDiagonal_ = Array(0);
            } else {
                throw Error("invalid size for tridiagonal operator "
                            "(must be null or >= 3)");
            }
        }

        TridiagonalOperator::TridiagonalOperator(
            const Array& low, const Array& mid, const Array& high)
        : diagonal_(mid), belowDiagonal_(low), aboveDiagonal_(high) {
            QL_ENSURE(low.size() == mid.size()-1,
                "wrong size for lower diagonal vector");
            QL_ENSURE(high.size() == mid.size()-1,
                "wrong size for upper diagonal vector");
        }

        void TridiagonalOperator::setLowerBC(
          const BoundaryCondition& bc) {
            lowerBC_ = bc;
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                setFirstRow(-1.0,1.0);
                break;
              case BoundaryCondition::Dirichlet:
                setFirstRow(1.0,0.0);
                break;
            }
        }

        void TridiagonalOperator::setUpperBC(
          const BoundaryCondition& bc) {
            upperBC_ = bc;
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                setLastRow(-1.0,1.0);
                break;
              case BoundaryCondition::Dirichlet:
                setLastRow(0.0,1.0);
                break;
            }
        }


        Array TridiagonalOperator::applyTo(const Array& v) const {
            QL_REQUIRE(v.size()==size(),
                "TridiagonalOperator::applyTo: vector of the wrong size (" +
                IntegerFormatter::toString(v.size()) + "instead of " +
                IntegerFormatter::toString(size()) + ")"  );
            Array result(size());

            // matricial product
            result[0] = diagonal_[0]*v[0] + aboveDiagonal_[0]*v[1];
            for (Size j=1;j<=size()-2;j++)
                result[j] = belowDiagonal_[j-1]*v[j-1]+ diagonal_[j]*v[j] +
                    aboveDiagonal_[j]*v[j+1];
            result[size()-1] = belowDiagonal_[size()-2]*v[size()-2] +
                diagonal_[size()-1]*v[size()-1];

            // apply lower boundary condition
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                result[0] = result[1] - lowerBC_.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[0] = lowerBC_.value();
                break;
            }

            // apply upper boundary condition
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                result[size()-1] = result[size()-2] + upperBC_.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[size()-1] = upperBC_.value();
                break;
            }

            return result;
        }

        Array TridiagonalOperator::solveFor(const Array& rhs) const {
            QL_REQUIRE(rhs.size()==size(),
                "TridiagonalOperator::solveFor: rhs has the wrong size");
            Array bcRhs = rhs;

            // apply lower boundary condition
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                bcRhs[0] = lowerBC_.value();
                break;
            }

            // apply upper boundary condition
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                bcRhs[size()-1] = upperBC_.value();
                break;
            }

            // solve tridiagonal system
            Array result(size()), tmp(size());

            double bet=diagonal_[0];
            QL_REQUIRE(bet != 0.0,
                "TridiagonalOperator::solveFor: division by zero");
            result[0] = bcRhs[0]/bet;
            Size j;
            for (j=1;j<=size()-1;j++){
                tmp[j]=aboveDiagonal_[j-1]/bet;
                bet=diagonal_[j]-belowDiagonal_[j-1]*tmp[j];
                QL_ENSURE(bet != 0.0,
                    "TridiagonalOperator::solveFor: division by zero");
                result[j] = (bcRhs[j]-belowDiagonal_[j-1]*result[j-1])/bet;
            }
            // cannot be j>=0 with Size j
            for (j=size()-2;j>0;j--)
                result[j] -= tmp[j+1]*result[j+1];
            result[0] -= tmp[1]*result[1];

            return result;
        }



        Array TridiagonalOperator::SOR(const Array& rhs, double tol) const {
            QL_REQUIRE(rhs.size()==size(),
                "TridiagonalOperator::solveFor: rhs has the wrong size");

            // initial guess
            Array result = rhs;

            // apply lower boundary condition
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                result[0] = lowerBC_.value();
                break;
            }

            // apply upper boundary condition
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                result[size()-1] = upperBC_.value();
                break;
            }

            // solve tridiagonal system with SOR technique
            Size sorIteration, i;
            double omega = 1.5;
            double err=2.0*tol;
            double temp;
            for (sorIteration=0; err>tol ; sorIteration++) {
                QL_REQUIRE(sorIteration<100000,
                    "TridiagonalOperator::SOR: tolerance ["
                    + DoubleFormatter::toString(tol) +
                    "] not reached in "
                    + IntegerFormatter::toString(sorIteration) +
                    " iterations. The error still is "
                    + DoubleFormatter::toString(err));
                err=0.0;
                for (i=1; i<size()-2 ; i++) {
                    temp = omega * (rhs[i]     -
                         aboveDiagonal_[i]   * result[i+1]-
                         diagonal_[i]        * result[i] -
                         belowDiagonal_[i-1] * result[i-1]) / diagonal_[i];
                    err += temp * temp;
                    result[i] += temp;
                }
            }

            return result;
        }

        TridiagonalOperator TridiagonalOperator::identity(Size size){
            return TridiagonalOperator(
                Array(size-1, 0.0),     // lower diagonal
                Array(size,   1.0),     // diagonal
                Array(size-1, 0.0));    // upper diagonal
        }

    }

}
